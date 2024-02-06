#ifndef PARSING_HPP
#define PARSING_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>

class   Location
{
    private :
        std::string    name;
        std::string    root;
        std::string    index;                             // not necessery
        std::string    upload_path;                       // not necessery
        bool           upload_enable;                     // not necessery
        std::map<std::string, std::string>    cgi;        // not necessery
        std::vector<std::string>              methods;    // not necessery
        bool           auto_index;                        // not necessery
        int            body_size;                         // not necessery

    public :
        Location();
        ~Location();

        void    set_location_name(const std::string& name);         //
        void    set_upload_path(const std::string& upload_path);    //
        void    set_upload_enable(bool upload_enable);              //
        void    set_root(std::string Root);                         //
        void    set_index(std::string Index);                       //
        void    set_methods(std::string Methods);                   //      Setters
        void    set_cgi(std::string CgiPath);                       //
        void    set_auto_index(bool AutoIndex);                     //
        void    set_body_size(int bodySize);                        //



        const std::string                               &get_location_name() const;         //   
        const std::string                               &get_upload_path() const;           //
        bool                                            get_upload_enable() const;          //
        const std::string                               &get_root() const;                  //
        const std::string                               &get_index() const;                 //
        std::vector<std::string>::iterator              get_methods_begin();                //
        std::vector<std::string>::iterator              get_methods_end();                  //      Getters
        std::map<std::string, std::string>::iterator    get_cgi_begin();                    //
        std::map<std::string, std::string>::iterator    get_cgi_end();                      //
        const bool                                      &get_auto_index() const;            //
        const int                                       &get_body_size() const;             //

        void    clear_cgi();
        void    clear_methods();
        bool    isEmpty(int flag);

};

class Server
{
    private :
        std::string                 name;
        int                         port;
        std::string                 root;
        std::map<int, std::string>  errors;     // not necessery

    public :
        Server();
        ~Server();

        std::vector<Location>       locations;

        void    set_root(const std::string& root);                  //
        void    set_name(std::string Name);                         //
        void    set_port(int Port);                                 //      Setters
        void    set_errors(std::pair<int, std::string> Errors);     //
        void    set_locations(Location Locations);                  //

        const std::string                             &get_name() const;            //
        const int                                     &get_port() const;            //
        const std::string                             &get_root() const;            //
        std::map<int, std::string>::iterator          get_errors_begin();           //      Getters
        std::map<int, std::string>::iterator          get_errors_end();             //
        std::vector<Location>::iterator               get_locations_begin();        //
        std::vector<Location>::iterator               get_locations_end();          //
        
        // void    print_all(std::vector<Server> vec); // Description: A function that print all the Attributes of the server class and of the location member class
};

std::vector<Server> ServerFill(std::string conf);

#endif