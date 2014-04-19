#ifndef UI_CONF_FILE_HPP
#define UI_CONF_FILE_HPP

#include "../logging/logger.hpp"
#include "ui_common.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <memory>
#include <type_traits>

namespace ui_conf_file
{
    static const std::string console_config_filename = "consoles_config.txt";
    typedef std::vector<std::string> string_vector;
    typedef std::pair<std::string,std::string> ss_pair;

    //Possible results of the read operation.
    typedef enum fileread_result_t
    {
        READ_OK,
        READ_READING_FAILURE,
        READ_PARSING_FAILURE
    }fileread_result_t;

    static const std::vector<std::pair<std::string,int>> instructions=
    {
        {"name",1}
    };

    static const std::pair<std::string,int> end_instruction{"end",0};

    static const std::vector<std::pair<std::string,int>> instruction_fields=
    {
        {"x_pos",0},
        {"y_pos",1},
        {"width",2},
        {"height",3},
        {"num_of_buttons",4},
        {"{",5},
        {"}",6},
        {"end",7},
    };

    //Reppresent an instruction
    class instruction
    {
        std::string m_name;
        std::map<std::string,std::string> m_fields;
    public:
        instruction(std::shared_ptr<string_vector> p_raw);
        int get_num_of_fields();
        std::string get_instruction_name();
        std::string get_instruction_value(const std::string& p_field="") ;
        string_vector get_instruction_fields();
    };


    //Responsible for the parsing activity
    typedef std::pair<std::string,std::string> strstr_pair;
    class instruction_parser
    {
        std::shared_ptr<string_vector> data;
        enum class parser_state{
            IDLE,
            READ,
            END
        }current_state;
        parser_state set_state(parser_state state);
        parser_state get_state();
        int search_instruction(const std::string& line);
        string_vector::iterator current_line_it;
    public:
        instruction_parser(std::shared_ptr<string_vector> raw_data);
        int continue_parsing();
    };

    typedef std::shared_ptr<instruction> instruction_ptr;
    //Instruction parsing
    class instruction_analyzer
    {
        std::vector<instruction_ptr> instructions;
        std::shared_ptr<string_vector> m_raw_data;
        long parse_raw_data();
    public:
        instruction_analyzer(std::shared_ptr<string_vector> p_raw_data);
        int instruction_count();
    };

    //Responsible for reading and parsing of the console file.
    class console_config_read
    {
    private:
        std::shared_ptr<string_vector> raw_data;
        std::string filename;
        int remove_comments(std::string& line);
    public:
        console_config_read(const std::string& config_filename);
        fileread_result_t read();
    };
}

#endif
