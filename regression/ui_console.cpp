#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#define private public

#include "..\config.hpp"
#include "ui_console.hpp"

#if COMPILE_TEST_MODE
/*
TEST(ConfigFileReading,configReadAndParse)
{
    ui_conf_file::console_config_read config(ui_conf_file::console_config_filename);
    //Read the configuration file
    ASSERT_EQ(ui_conf_file::fileread_result_t::READ_OK,config.read());
    //Check the lenght of the raw data file.
    ASSERT_EQ(config.raw_data->size(),26);
}
/*
TEST(ConfigInstructionAnalyzer,instructionParsingCheckAmount)
{
    ui_conf_file::console_config_read config(ui_conf_file::console_config_filename);
    //Read the configuration file
    ASSERT_EQ(ui_conf_file::fileread_result_t::READ_OK,config.read());
    //Init che instruction_analyzer
    ui_conf_file::instruction_analyzer analyzer(config.raw_data);
    //Check the amount of instructions
    ASSERT_EQ(analyzer.instruction_count(),2);
}*/

TEST(InstructionParser,basicParsing)
{
    ui_conf_file::console_config_read config(ui_conf_file::console_config_filename);
    //Read the configuration file
    ASSERT_EQ(ui_conf_file::fileread_result_t::READ_OK,config.read());
    //Use the instruction parser
    ui_conf_file::instruction_parser instr_parser(config.raw_data);
    //Read the first instruction
    int first_instr=instr_parser.continue_parsing();
    ASSERT_EQ(first_instr,1);
    int second_instr=instr_parser.continue_parsing();
    ASSERT_EQ(second_instr,1);
}

#endif
