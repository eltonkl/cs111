#include "tokens.h"

bool validate_option(option_t* opt)
{
	//If none of the options are chosen then we don't have a valid option
	if (opt->type < 0 || opt->type > 4) return false;
	
	//Otherwise, we must check to make sure that the option has the appropriate number of arguments
	if(opt->type == file_flag_ && opt->num_args != 0) return false;									//All flags have no args
	if(opt->type == file_open_)																		//All opens have one arg, except pipe which has no args
	{
		if((opt->option == rdwr_ || opt->option == rdonly_ || opt->option == wronly_) && opt->num_args != 1) return false;
		if(opt->option == pipe_ && opt->num_args != 0) return false;
	}
	
	if(opt->type == subcommand_)																	//command has i e o cmd and 0 or more args, wait has no args
	{
		if(opt->option == command_ && opt->num_args < 4) return false;
		if(opt->option == wait_ && opt->num_args != 0) return false;
	}
	
	if(opt->type == misc_)																			//Some misc have no args, some have one arg
	{
		if ((opt->option == catch_ || opt->option == ignore_ || opt->option == default_) && opt->num_args != 1) return false;
		if ((opt->option == verbose_ || opt->option == profile_ || opt->option == abort_ || opt->option == pause_) && opt->num_args != 0) return false;
	}
	
	
	//If we make it this far, then our option is indeed valid.
	return true;
}