#pragma once
#include "../~INCs/HTTPrelated.hpp"
#include "../Exposed_Functions.hpp"

void        Print_generic_Rules(void);
void        _clear(Client &_client);
std::string generic_error_page(unsigned int error_code, std::string phrase);
std::string generic_index_page(std::string URI_);