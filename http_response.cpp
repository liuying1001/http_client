#include "http_response.h"

#include <iostream>

HttpResponse::HttpResponse() :
    m_location(""),
    m_status_code(0),
    m_content_length(0)
{

}

HttpResponse::~HttpResponse()
{

}

//the response_header is end with \r\n\r\n
//so after it, that the file_data client need to download
//this function only obtain the response_header to judge to whether client request successfully


void HttpResponse::parse_response(const std::string& response)
{
    parse_status_code(response);
    parse_location(response);
    parse_content_length(response);
}


void HttpResponse::parse_status_code(const std::string& response)
{
    std::string::size_type front_idx = 0;
    std::string::size_type back_idx = 0;
    while((front_idx = response.find("HTTP/1.1", front_idx)) != std::string::npos)
    {
        front_idx += 8;
        while(response.at(front_idx) == ' ')
            ++front_idx;
        back_idx = response.find(" ", front_idx);
        std::string status_code = response.substr(front_idx, back_idx - front_idx);
        sscanf(status_code.c_str(), "%d", &m_status_code);
    }
}

void HttpResponse::parse_location(const std::string& response)
{
    m_location = "";
    if(m_status_code == 302)
    {
        std::string::size_type front_idx = response.find("Location:");
        if(front_idx == std::string::npos)
            return ;

        front_idx += 9;
        while(response[front_idx] == ' ')
            ++front_idx;

        std::string::size_type back_idx = response.find("\r\n", front_idx);
        m_location = response.substr(front_idx, back_idx - front_idx);
   }
}


//if request successfully, the response_header will include "Content-Length: " 
//client can use to number follow closely it to know the file size
void HttpResponse::parse_content_length(const std::string& response)
{
    m_content_length = 0;
    std::string::size_type front_idx = response.find("Content-Length: ");
    if(front_idx == std::string::npos)
        return;
    
    front_idx += 15;
    while(response[front_idx] == ' ')
        ++front_idx;
    std::string::size_type back_idx = response.find("\r\n", front_idx);
    
    std::string content_length_str = response.substr(front_idx, back_idx - front_idx);
    sscanf(content_length_str.c_str(), "%lld", &m_content_length);
}


HTTP_STATUS HttpResponse::get_status_code() const
{
    if(m_status_code == 200)
        return HTTP_STATUS_OK;
    else if(m_status_code == 302)
        return HTTP_STATUS_REDIR;
    else if(m_status_code == 400)
        return HTTP_STATUS_BAD;
    else if(m_status_code == 404)
        return HTTP_STATUS_NFOUND;
    else
        return HTTP_STATUS_OTHER;
}

std::string HttpResponse::get_location() const
{
    return m_location;
}

long long int HttpResponse::get_content_length() const
{
    return m_content_length;
}
//judge whether request ok or not
//and whether redirection or not
//
//there is a problem, if redirection, and after use the new url to download the file
//the response will include both 302(redirection) and 200(successful)
//in this case, the project regard it as successful(200) and start download the file
//


std::string HttpResponse::get_http_response(int status_code, const std::string& filename)
{
    if(status_code == 200)
        return get_response_200(filename);
    else if(status_code == 302)
        return get_response_302();
    else if(status_code == 400)
        return get_response_400();
    else
        return "";
}

std::string HttpResponse::get_response_200(const std::string& filename)
{
    struct stat fileinfo;
    lstat(filename.c_str(), &fileinfo);
  
    time_t timer;
    time(&timer);
    std::stringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << ctime(&timer) << "\r\n"
        << "Content-Length: " << fileinfo.st_size << "\r\n"
        << "\r\n";
   return oss.str();; 
}

std::string HttpResponse::get_response_302()
{
    return "";
}

std::string HttpResponse::get_response_400()
{
    return "";
}
