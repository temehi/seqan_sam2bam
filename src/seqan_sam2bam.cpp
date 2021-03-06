// ==========================================================================
//    SEQAN_SAM2BAM - Species Level Identification of Microbes from Metagenomes.
// ==========================================================================
// Copyright (c) 2014-2017, Temesgen H. Dadi, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Temesgen H. Dadi or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL TEMESGEN H. DADI OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Temesgen H. Dadi <temesgen.dadi@fu-berlin.de>
// ==========================================================================
#include <seqan/basic.h>
#include <seqan/file.h>
#include <seqan/sequence.h>
#include <seqan/arg_parse.h>
#include <seqan/seq_io.h>

#include <string>
#include <iostream>

using namespace seqan;


struct AppOptions
{
    typedef std::vector<std::string>            TDirection;
    TDirection          dirList     = {"b2s", "s2b"};

    std::string         input_path;
    std::string         output_path;
    std::string         direction;

    AppOptions() :
    input_path(""),
    output_path(""),
    direction("s2b")
    {}
};

// ----------------------------------------------------------------------------
// Function setDateAndVersion()
// ----------------------------------------------------------------------------

void setDateAndVersion(ArgumentParser & parser)
{
    setDate(parser, __DATE__);
    setShortDescription(parser, "Converts a SAM file to BAM format");
    setCategory(parser, "Utility");
#if defined(SEQAN_APP_VERSION)
    setVersion(parser, SEQAN_APP_VERSION);
#endif
}


std::string getOutFileName (const std::string& inpfName, const std::string& dir)
{
    std::string result = inpfName;
    std::string oldExt = ".sam";
    std::string newExt = ".bam";
    if (dir.compare("b2s") == 0)
    {
        oldExt = ".bam";
        newExt = ".sam";
    }

    if (result.find_last_of(oldExt) != std::string::npos && result.find_last_of(oldExt) == result.find_last_of("."))
        result.replace((result.find_last_of(".")), 4, "");
    result.append(newExt);
    return result;
}

// --------------------------------------------------------------------------
// Function parseCommandLine()
// --------------------------------------------------------------------------
ArgumentParser::ParseResult
parseCommandLine(ArgumentParser & parser, AppOptions & options, int argc, char const ** argv)
{
    // Setup ArgumentParser.
    setDateAndVersion(parser);
    // Define usage line and long description.
    addUsageLine(parser, "[\\fIOPTIONS\\fP] \"\\fIIN\\fP\"");

    // The input file/directory argument.
    addArgument(parser,
                ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));

    // The output file argument.
    addOption(parser,
              ArgParseOption("o", "output-path", "the path to output file.",
                             ArgParseArgument::OUTPUT_FILE));
    addOption(parser,
              ArgParseOption("d", "direction",
                             "Conversion direction", ArgParseOption::STRING));
    setValidValues(parser, "direction", options.dirList);
    setDefaultValue(parser, "direction", options.direction);


    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    // Extract Argument values.
    getArgumentValue(options.input_path, parser, 0);

    // Extract option values.
    getOptionValue(options.output_path, parser, "output-path");
    getOptionValue(options.direction, parser, "direction");
    if (!isSet(parser, "output-path"))
        options.output_path = getOutFileName(options.input_path, options.direction);


    return ArgumentParser::PARSE_OK;
}

// --------------------------------------------------------------------------
// Function main()
// --------------------------------------------------------------------------

// Program entry point.
int main(int argc, char const ** argv)
{
    // Parse the command line.
    ArgumentParser parser("seqan_sam2bam");
    AppOptions options;
    ArgumentParser::ParseResult res = parseCommandLine(parser, options, argc, argv);

    // If there was an error parsing or built-in argument parser functionality
    // was triggered then we exit the program.  The return code is 1 if there
    // were errors and 0 if there were none.

    if (res != ArgumentParser::PARSE_OK)
        return res == ArgumentParser::PARSE_ERROR;

    BamFileIn bam_file_in;

    if (!open(bam_file_in, toCString(options.input_path)))
    {
        std::cerr << "Could not open " << options.input_path << "!\n";
        return 1;
    }

    BamFileOut bam_file_out(context(bam_file_in), toCString(options.output_path));

    // Read the header from the input file first and and write it to the output file
    BamHeader header;
    readHeader(header, bam_file_in);
    writeHeader(bam_file_out, header);

    // Iterate over the records and copy them from the input file to the output file
    while (!atEnd(bam_file_in))
    {
        BamAlignmentRecord record;
        readRecord(record, bam_file_in);
        writeRecord(bam_file_out, record);
    }

    close(bam_file_in);
    close(bam_file_out);

    return 0;
}
