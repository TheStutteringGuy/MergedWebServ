#include "WebServer.hpp"

pid_t Client::Handle_CGI(const std::string bin, const std::string actual_URI, www::fd_t *sv)
{

    pid_t pid;
    pid = fork();
    if (pid == -1)
        std::exit(127);
    else if (pid == 0)
    {
        close(sv[0]);
        if (/*dup2(sv[1], STDIN_FILENO) == -1 ||*/ dup2(sv[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "dup2() failed !!" << std::endl;
            std::exit(1);
        }
        close(sv[1]);

        std::string cookie_data = this->serialize_cookies();

        std::vector<std::string> env_strings;
        env_strings.push_back("REQUEST_METHOD=" + this->m_request.m_method);
        env_strings.push_back("QUERY_STRING=" + this->m_request.m_queryString);

        if (!cookie_data.empty())
            env_strings.push_back("HTTP_COOKIE=" + cookie_data);

        if (this->m_request.m_method == "POST")
        {
            const std::vector<std::string> *content_lenght_header = find_Value_inMap(this->m_request.m_headers, "Content-Length");

            if (NULL != content_lenght_header && !(*content_lenght_header).empty())
            {
                std::string content_lenght;

                content_lenght = (*content_lenght_header)[0];
                env_strings.push_back("CONTENT_LENGTH=" + content_lenght);
            }
            else
                return 0; // i don't know what to return exactly :)
        }

        std::vector<char *> env_p;
        for (size_t i = 0; i < env_strings.size(); ++i)
            env_p.push_back(const_cast<char *>(env_strings[i].c_str()));
        env_p.push_back(NULL);
        char *const *envp = env_p.data();
        char *const argv[] = {const_cast<char *>(bin.c_str()), const_cast<char *>(actual_URI.c_str()), NULL};

        if (execve(bin.c_str(), argv, envp) == -1)
        {
            std::cerr << "execve() failed for CGI: " + static_cast<std::string>(strerror(errno)) << std::endl;
            std::exit(127);
        }
        std::exit(0);
        return 0;
    }
    else
        return pid;
}