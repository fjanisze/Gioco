#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_conf_file.hpp"
#include <iostream>

namespace ui_conf_file
{

    console_config_read::console_config_read(const std::string& config_filename) :
            filename(config_filename), raw_data(new string_vector)
    {
        LOG("console_config_read::console_config_read(): Filename: ",filename);
    }

    //Read the configuration file and prepare the raw data for the parsing
    fileread_result_t console_config_read::read()
    {
        LOG("fileread_result_t console_config_read(): Start reading");
        fileread_result_t result=fileread_result_t::READ_OK;
        std::ifstream in_file(filename.c_str());
        if(in_file)
        {
            std::string line;
            while(std::getline(in_file,line))
            {
                //Remove the tabs and spaces if any
                int i=0;
                for(;i<line.size();i++)
                {
                    if(line[i]!=' '&&
                       line[i]!='\t') break;
                }
                line.erase(0,i);
                if(line.size())
                {
                    //Check and remove comment lines
                    if(remove_comments(line)==0)
                    {
                        continue;
                    }
                    raw_data->push_back(line);
                }
            }
            ELOG("console_config_read::read(): Amount of raw lines read: ",raw_data->size());
        }
        else
        {
            LOG_ERR("console_config_read::read(): Reading failure!");
            result=fileread_result_t::READ_READING_FAILURE;
        }
        return result;
    }

    //Remove from the given line all the comments
    int console_config_read::remove_comments(std::string& line)
    {
        auto it=line.begin();
        auto end=line.end();
        while(it!=end)
        {
            if(*it=='#') break;
            ++it;
        }
        if(it!=end)
        {
            line.erase(it,end);
        }
        return line.size();
    }

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for instruction
	//
	//
	////////////////////////////////////////////////////////////////////

    instruction::instruction(std::shared_ptr<string_vector> p_raw)
    {
        for(auto e:*p_raw)
        {
            std::cout<<e<<std::endl;
        }
        p_raw->clear();
    }

    int instruction::get_num_of_fields()
    {
        return m_fields.size();
    }

    std::string instruction::get_instruction_name()
    {
        return m_name;
    }

    std::string instruction::get_instruction_value(const std::string& p_field)
    {
        if(p_field.empty())
        {
            return m_fields[m_name];
        }
        return m_fields[p_field];
    }

    string_vector instruction::get_instruction_fields()
    {
        //todo
    }

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for instruction_parser and instruction_analyzer
	//
	//
	////////////////////////////////////////////////////////////////////

    instruction_analyzer::instruction_analyzer(std::shared_ptr<string_vector> p_raw_data):
                m_raw_data(p_raw_data)
    {
        if(m_raw_data->empty())
        {
            LOG_ERR("instruction_analyzer::instruction_analyzer(): raw_data is empty!");
            throw std::runtime_error("raw_data is empty, not possible to proceed with the parsing");
        }
        long instr_count=parse_raw_data();
        LOG("instruction_analyzer::instruction_analyzer(): Amount of instructions:",instr_count);
    }

    //Parse the raw data and fill the instructions container
    long instruction_analyzer::parse_raw_data()
    try{
        LOG("instruction_analyzer::parse_raw_data(): Starting");
        //TODO
        return instructions.size();
    }catch(std::exception& xa)
    {
        LOG_ERR("instruction_analyzer::parse_raw_data(): Parsing failure, what: ",xa.what());
        return -1;
    }

    //Return the amount of instructions
    int instruction_analyzer::instruction_count()
    {
        return instructions.size();
    }

    instruction_parser::instruction_parser(std::shared_ptr<string_vector> raw_data)
    {
        //Make a local copy
        data.reset(new string_vector(*raw_data));
        ELOG("instruction_parser::instruction_parser(): State IDLE, amount of data: ",data->size());
        set_state(parser_state::IDLE);
        //Set the current line iterator
        current_line_it=raw_data->begin();
    }

    //Return the ID number for the instruction, if any has been found.
    int instruction_parser::search_instruction(const std::string& line)
    {
        for(auto instr:instructions)
        {
            size_t pos=line.find(instr.first);
            if(pos!=std::string::npos)
            {
                //Check if this instruction is surrounded by @
                bool begin_with_at=false,
                    end_with_at=false;
                if(pos>0)
                {
                    if(line[pos-1]=='@')
                        begin_with_at=true;
                }
                size_t tmp_pos=pos+instr.first.size();
                if(tmp_pos<line.size())
                {
                    if(line[tmp_pos]=='@')
                        end_with_at=true;
                }
                if(begin_with_at&&end_with_at)
                {
                    ELOG("instruction_parser::search_instruction(): The line\"",line,"\" contain the instruction: \"",instr.first,"\"");
                    return instr.second;
                }
            }
        }
        return -1;
    }

    //Return the ID of the instruction or -1 if any has been found
    int instruction_parser::continue_parsing()
    {
        if(data->empty()) return false; //Nothing to parse
        set_state(parser_state::READ);
        //Look for the next instruction
        while(current_line_it!=data->end())
        {
            int index=search_instruction(*current_line_it);
            if(index>0)
            {
                return index;
            }
            ++current_line_it;
        }
        return -1;
    }

    instruction_parser::parser_state instruction_parser::set_state(parser_state state)
    {
        parser_state cur=current_state;
        current_state=state;
        return cur;
    }

    instruction_parser::parser_state instruction_parser::get_state()
    {
        return current_state;
    }
}






























