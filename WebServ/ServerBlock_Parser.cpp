#include "Exposed_Functions.hpp"
#include "~INCs/HTTPrelated.hpp"
#include "~INCs/Namespaces.hpp"

static std::string root_check(std::string& root)
{
    return std::string(root);

    // std::string tmp(root);
    // if (tmp[tmp.size() - 1] != '/')
    //     tmp += '/';
    // return tmp;
}

static std::map<std::string, std::vector<std::string> > directives_tomap(std::vector<Directive> &directives)
{
    std::map<std::string, std::vector<std::string> > tmp;

    for (size_t index = 0; index < directives.size(); ++index)
    {
        // std::cout << directives[index].name;
        tmp[directives[index].name];
        for (size_t i = 0; i < directives[index].values.size() ; ++i)
        {
            // std::cout << " " << directives[index].values[i];
            tmp[directives[index].name].push_back(directives[index].values[i]);
        }
        // std::cout << std::endl;
    }
    return tmp;
}

static std::map<std::string, MyLocationBlock> handle_LocationBlocks(std::vector<LocationBlock>& locationBlocks)
{
    std::map<std::string, MyLocationBlock> tmp;

    for (size_t index = 0; index < locationBlocks.size(); ++index)
    {
        tmp[locationBlocks[index].path];

        tmp[locationBlocks[index].path].directives = directives_tomap(locationBlocks[index].directives);
        tmp[locationBlocks[index].path].root = root_check(locationBlocks[index].root);
        tmp[locationBlocks[index].path].index = locationBlocks[index].index;
        tmp[locationBlocks[index].path].autoindex = locationBlocks[index].autoindex;
        tmp[locationBlocks[index].path].allowed_methods = locationBlocks[index].allowed_methods;
        tmp[locationBlocks[index].path].is_Red = locationBlocks[index].is_redirection;
        tmp[locationBlocks[index].path].redirection = locationBlocks[index].redirect_url;
        tmp[locationBlocks[index].path].is_CGI = locationBlocks[index].is_cgi;
        tmp[locationBlocks[index].path].cgi_infos = locationBlocks[index].cgi;
        tmp[locationBlocks[index].path].is_Upload = locationBlocks[index].upload;
        tmp[locationBlocks[index].path].uplaod_path = locationBlocks[index].upload_path;
    }
    return tmp;
}

void API::ServerBlock_Parser(std::vector<ServerBlock> &servers_blocks)
{
    size_t servers = servers_blocks.size();
    std::vector<MyServerBlock>& ref = ValuesSingleton::getValuesSingleton().servers_blocks;

    for (size_t index = 0; index < servers; ++index)
    {
        ref.push_back(MyServerBlock());

        ref[index].m_locationBlocks = handle_LocationBlocks(servers_blocks[index].locationBlocks);
        ref[index].m_root = root_check(servers_blocks[index].root);
        ref[index].m_cache = ref[index].m_root + "/" + static_cast<std::string>(cache);
        ref[index].m_error_pages = servers_blocks[index].error_pages;
        ref[index].m_directives = directives_tomap(servers_blocks[index].directives);
        ref[index].m_client_max_body_size = servers_blocks[index].clientSizeBody;
    }
};