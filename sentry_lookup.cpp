#include <cstdio>
#include <iostream>
#include <fstream>
#include <regex>
#include <unistd.h>
#include "libini.h"

// [{"required_test":0,"date":"Saturday, August 18, 2018  5:26AM MST","text":"Do not test today","transaction_key":"ffffffff"}]

#define DEFAULT_CONF    "users.conf"
#define DEFAULT_TRIES   10
#define DEFAULT_SLEEP   15
#define VERSION         "0.1.5"

std::string cap(std::string input);
bool stob(const std::string &str);
std::string btos(bool cond);
int handleSwitches(INIObject &settings, std::string config, int argc, char *argv[]);
void versionInfo();
void helpInfo(char *cmd);
void usageInfo(char *cmd);

bool notifyOff          =   false;
bool verbose            =   false;
std::string logFile     =   "";

int main(int argc, char *argv[])
{
    INIObject conf;
    int err;
    if ((err = handleSwitches(conf,DEFAULT_CONF,argc,argv)))
    {
        if (err == 1)
            return 0;
        return 1;
    }
    //INIObject conf (DEFAULT_CONF);
    auto settings = conf.topic_it("settings");
    std::string def_lang = settings->find("lang");
    bool def_notifyNoTest = stob(settings->find("notify_regardless"));
    bool def_notifyError = stob(settings->find("notify_error"));
    int def_tries = std::stoi(settings->find("tries"));
    int def_sleep = std::stoi(settings->find("sleep_time"));
    if (def_sleep > 0)
        def_sleep--;
    conf.erase(settings);
    bool needSleep = false;
    
    for (auto it = conf.begin(), ite = conf.end();it != ite;++it)
    {
        std::string lang = def_lang;
        bool notifyNoTest = def_notifyNoTest, notifyError = def_notifyError;
        if (it->exists("lang"))
            lang = (*it)("lang");
        if (it->exists("notify_regardless"))
            notifyNoTest = stob((*it)("notify_regardless"));
        if (it->exists("notify_error"))
            notifyError = stob((*it)("notify_error"));
        std::string email = it->topic();
        std::string name = (*it)("name");
        std::string cmd = "curl -s -d \"phone=" + (*it)("phone") + "&last_name=" + name + "&ivr_code=" + (*it)("id") + "&lang=" + lang + "\" -X POST https://sentry.cordanths.com/Sentry/WebCheckin/Log > result.out";
        if (it->exists("nickname"))
            name = (*it)("nickname");
        else
            name = cap(name);
        if (it->exists("prefix"))
            name = (*it)("prefix") + " " + name;
        std::string data;
        for (int tries = def_tries;tries > 0;--tries)
        {
            if (needSleep)
                sleep(def_sleep);
            else if (def_sleep)
                needSleep = true;
            system(cmd.c_str());
            sleep(1);
            std::ifstream result ("result.out");
            if (result.is_open())
            {
                result.seekg(0, std::ios::end);   
                data.reserve(result.tellg());
                result.seekg(0, std::ios::beg);
                data.assign((std::istreambuf_iterator<char>(result)),std::istreambuf_iterator<char>());
                result.close();
                data = data.substr(0,data.find('\r'));
            }
            if (data.size() > 0)
                break;
        }
        if (data.size() > 0)
        {
            if (logFile.size() > 0)
            {
                std::ofstream logOut (logFile,std::ofstream::app);
                if (logOut.is_open())
                {
                    logOut<<name<<":"<<email<<"<<"<<data<<std::endl;
                    logOut.close();
                }
            }
            
            std::regex ptrn ("\"error_msg\":\\s*\"(.+?)\"");
            std::smatch ml;
            if (std::regex_search(data,ml,ptrn))
            {
                if ((notifyError) && (!notifyOff) && (email != "-"))
                    system(("echo \"" + ml[1].str() + "\" | mail -s \"" + name + ", an Error has Occured!\" " + email).c_str());
                if ((verbose) || (email == "-"))
                    std::cout<<name<<": "<<ml[1].str()<<std::endl<<std::endl;
            }
            else
            {
                std::regex ptrn0 ("\"text\":\\s*\"(.+?)\"");
                std::regex ptrn1 ("\"transaction_key\":\\s*\"(.+?)\"");
                std::regex ptrn2 ("\"date\":\\s*\"(.+?)\"");
                std::regex ptrn3 ("\"required_test\":\\s*(.+?)[,}]");
                std::string msg;
                if (std::regex_search(data,ml,ptrn0))
                    msg = ml[1].str();
                if (std::regex_search(data,ml,ptrn1))
                    msg = msg + "\nTransaction ID: " + ml[1].str();
                if (std::regex_search(data,ml,ptrn2))
                    msg = msg + "\n" + ml[1].str();
                std::string test = "error";
                if (std::regex_search(data,ml,ptrn3))
                    test = ml[1].str();
                if ((!notifyOff) && (email != "-"))
                {
                    if (test == "0")
                    {
                        if (notifyNoTest)
                            system(("echo \"" + msg + "\" | mail -s \"Nothing to see here, " + name + "\" " + email).c_str());
                    }
                    else if (test == "error")
                        system(("echo \"Oops! Cannot determine if you need to test today.\n" + msg + "\" | mail -s \"" + name + ", an Error has Occured!\" " + email).c_str());
                    else
                        system(("echo \"" + msg + "\" | mail -s \"Drink it up " + name + "! It's time to pee!\" " + email).c_str());
                }
                if ((verbose) || (email == "-"))
                    std::cout<<name<<": "<<msg<<std::endl<<std::endl;
            }
            remove("result.out");
        }
        else
        {
            if ((!notifyOff) && (notifyError) && (email != "-"))
                system(("echo \"Failed to connect to the sentry testing server after " + std::to_string(def_tries) + " tries!\" | mail -s \"" + name + ", an Error has Occured!\" " + email).c_str());
            if ((verbose) || (email == "-"))
                std::cout<<name<<": Failed to connect to the sentry testing server after "<<def_tries<<" tries!\n"<<std::endl;
        }
    }
    return 0;
}

std::string cap(std::string input)
{
    if (input.size() > 0)
    {
        input.front() = toupper(input.at(0));
        for (auto it = input.begin()+1, ite = input.end();it != ite;++it)
            *it = tolower(*it);
    }
    return input;
}

bool stob(const std::string &str)
{
    if (str == "true")
        return true;
    if (isdigit(str.front()))
        return stoi(str);
    return false;
}

std::string btos(bool cond)
{
    if (cond)
        return "true";
    return "false";
}

int handleSwitches(INIObject &settings, std::string config, int argc, char *argv[])
{
    bool        noConf      =   false;
    std::string lang        =   "en";
    bool        noTest      =   false;
    bool        error       =   true;
    bool        save        =   false;
    bool        addUser     =   false;
    bool        single      =   false;
    int         tries       =   DEFAULT_TRIES;
    int         sleep       =   DEFAULT_SLEEP;
    bool        langSet     =   false;
    bool        noTestSet   =   false;
    bool        errorSet    =   false;
    bool        noLookup    =   false;
    bool        triesSet    =   false;
    bool        sleepSet    =   false;
    int arg = 1;
    for (;arg < argc;arg++)
    {
        if (std::string(argv[arg]).compare(0,2,"--") == 0)
        {
            if (strcmp(argv[arg],"--") == 0)
            {
                arg++;
                break;
            }
        }
        else
            break;
        if (strcmp(argv[arg],"--version") == 0)
        {
            versionInfo();
            return 1;
        }
        if (strcmp(argv[arg],"--help") == 0)
        {
            helpInfo(argv[0]);
            return 1;
        }
        else if (strcmp(argv[arg],"--no-config") == 0)
            noConf = true;
        else if (strcmp(argv[arg],"--config") == 0)
        {
            if (++arg >= argc)
            {
                std::cerr<<"Command line switch '--config' missing config file argument. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
            config = argv[arg];
        }
        else if (strcmp(argv[arg],"--verbose") == 0)
            verbose = true;
        else if (strcmp(argv[arg],"--save") == 0)
            save = true;
        else if (strcmp(argv[arg],"--add-user") == 0)
            addUser = true;
        else if (strcmp(argv[arg],"--single") == 0)
            single = true;
        else if (strcmp(argv[arg],"--lang") == 0)
        {
            if (++arg >= argc)
            {
                std::cerr<<"Command line switch '--lang' missing language argument. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
            lang = argv[arg];
            langSet = true;
        }
        else if (strcmp(argv[arg],"--notify-off") == 0)
            notifyOff = true;
        else if (strcmp(argv[arg],"--notify-regardless") == 0)
            noTest = noTestSet = true;
        else if (strcmp(argv[arg],"--notify-regardless-off") == 0)
        {
            noTest = false;
            noTestSet = true;
        }
        else if (strcmp(argv[arg],"--notify-error-off") == 0)
        {
            error = false;
            errorSet = true;
        }
        else if (strcmp(argv[arg],"--notify-error") == 0)
        {
            error = true;
            errorSet = true;
        }
        else if (strcmp(argv[arg],"--no-lookup") == 0)
            noLookup = true;
        else if (strcmp(argv[arg],"--log") == 0)
        {
            if (++arg >= argc)
            {
                std::cerr<<"Command line switch '--log' missing log file argument. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
            logFile = argv[arg];
        }
        else if (strcmp(argv[arg],"--tries") == 0)
        {
            if (++arg >= argc)
            {
                std::cerr<<"Command line switch '--tries' missing digit argument. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
            bool gotSet = isdigit(argv[arg][0]);
            if (gotSet)
            {
                tries = atoi(argv[arg]);
                if (tries < 1)
                    gotSet = false;
                triesSet = true;
            }
            if (!gotSet)
            {
                std::cerr<<"Command line switch '--tries': Invalid digit: '"<<argv[arg]<<"'. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
        }
        else if (strcmp(argv[arg],"--sleep-time") == 0)
        {
            if (++arg >= argc)
            {
                std::cerr<<"Command line switch '--sleep-time' missing seconds argument. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
            bool gotSet = isdigit(argv[arg][0]);
            if (gotSet)
            {
                sleep = atoi(argv[arg]);
                if (sleep < 0)
                    gotSet = false;
                sleepSet = true;
            }
            if (!gotSet)
            {
                std::cerr<<"Command line switch '--sleep-time': Invalid seconds argument: '"<<argv[arg]<<"'. See '"<<argv[0]<<" --help'."<<std::endl;
                return 2;
            }
        }
        else
            std::cerr<<"Ignoring unknown command switch '"<<argv[arg]<<"'. See '"<<argv[0]<<" --help'."<<std::endl;
    }
    std::string userData[4];
    if (argc-arg == 4)
        for (int i = 0;arg < argc;arg++)
            userData[i++] = argv[arg];
    else if (argc-arg != 0)
    {
        usageInfo(argv[0]);
        return 2;
    }
    
    if (!noConf)
    {
        if (settings.open(config))
        {
            std::cerr<<"Could not load config file: '"<<config<<"'"<<std::endl;
            return 2;
        }
        if (!settings.exists("settings"))
            settings("settings","lang");
        auto opt = settings.topic_it("settings");
        if ((langSet) || (!opt->exists("lang")))
            (*opt)("lang") = lang;
        if ((noTestSet) || (!opt->exists("notify_regardless")))
            (*opt)("notify_regardless") = btos(noTest);
        if ((errorSet) || (!opt->exists("notify_error")))
            (*opt)("notify_error") = btos(error);
        if ((triesSet) || (!opt->exists("tries")))
            (*opt)("tries") = std::to_string(tries);
        else
        {
            std::string t = opt->find("tries");
            if ((!isdigit(t[0])) || (std::stoi(t) < 1))
                (*opt)("tries") = std::to_string(DEFAULT_TRIES);
        }
        if ((sleepSet) || (!opt->exists("sleep_time")))
            (*opt)("sleep_time") = std::to_string(sleep);
        else
        {
            std::string s = opt->find("sleep_time");
            if ((!isdigit(s[0])) || (std::stoi(s) < 0))
                (*opt)("sleep_time") = std::to_string(DEFAULT_SLEEP);
        }
    }
    else
    {
        settings("settings","lang") = lang;
        auto opt = settings.topic_it("settings");
        (*opt)("notify_regardless") = btos(noTest);
        (*opt)("notify_error") = btos(error);
        (*opt)("tries") = std::to_string(tries);
        (*opt)("sleep_time") = std::to_string(sleep);
    }
    if (save)
    {
        if (settings.write(config))
            std::cerr<<"Unable to save settings to '"<<config<<"'."<<std::endl;
        else
            std::cout<<"Settings successfully saved to '"<<config<<"'."<<std::endl;
    }
    if (userData[0].size() > 0)
    {
        if (addUser)
        {
            std::ofstream file (config,std::ofstream::app);
            if (file.is_open())
            {
                file<<"\n["<<userData[0]<<"]\nphone="<<userData[1]<<"\nname="<<userData[2]<<"\nid="<<userData[3]<<'\n';
                if (langSet)
                    file<<"lang="<<lang<<'\n';
                if (noTestSet)
                    file<<"notify_regardless="<<btos(noTest)<<'\n';
                if (errorSet)
                    file<<"notify_error="<<btos(error)<<'\n';
                file.close();
                std::cout<<"User successfully added to '"<<config<<"'."<<std::endl;
            }
            else
                std::cerr<<"Unable to add user to '"<<config<<"'."<<std::endl;
        }
        if (single)
        {
            for (auto it = settings.begin();it != settings.end();)
            {
                if (it->topic() == "settings")
                    ++it;
                else
                    it = settings.erase(it);
            }
        }
        size_t skip = settings.topics(userData[0]);
        settings(userData[0],"phone",skip) = userData[1];
        auto opt = settings.topic_it(userData[0],skip);
        (*opt)("name") = userData[2];
        (*opt)("id") = userData[3];
    }
    else if (single)
    {
        std::cerr<<"Error: '--single' switch set without providing user info.\n";
        usageInfo(argv[0]);
        return 2;
    }
    if (noLookup)
        return 1;
    return 0;
}

void versionInfo()
{
    std::cout<<"\nVERSION\n\tAutomated Sentry Testing Notification System v"<<VERSION<<" by nigel.\n"<<std::endl;
}

void helpInfo(char *cmd)
{
    versionInfo();
    std::cout<<"DESCRIPTION\n\tThis program will lookup drug testing information for an individual or\n\tset of individuals and email the results.\n";
    usageInfo(cmd);
    std::cout<<"OPTIONS\n"
             <<"\t--\t\t\tMark the end of the switches. Useful if your\n\t\t\t\temail begins with '--'.\n\n"
             <<"\t--help\t\t\tDisplay this help message and exit.\n\n"
             <<"\t--version\t\tOutput the version and exit.\n\n"
             <<"\t--add-user\t\tSave the user information provided to the\n\t\t\t\tconfiguration file.\n\n"
             <<"\t--config\t\tSet the configuration file to the argument\n\t\t\t\tdirectly following this switch.\n\n\t\t`"<<cmd<<" --config \"~/path/to/config\"`\n\n"
             <<"\t--lang\t\t\tSet the language, valid options: 'en' or 'es'.\n\t\t\t\t(Default: en)\n\n"
             <<"\t--no-config\t\tDo not load the config file. If settings are not\n\t\t\t\tdefined with switches the defaults will be used.\n\n"
             <<"\t--no-lookup\t\tDoes not actually lookup any test results,\n\t\t\t\tuseful for modifying the configuration file\n\t\t\t\twithout running the program.\n\n"
             <<"\t--notify-error\t\tEnables notifying on error. (Default)\n\n"
             <<"\t--notify-error-off\tDisables notifying on error.\n\n"
             <<"\t--notify-off\t\tDisables all email notifications for the current\n\t\t\t\tsession.\n\n"
             <<"\t--notify-regardless\tEnables notifications even when no test is\n\t\t\t\trequired.\n\n"
             <<"\t--notify-regardless-off\tDisables notifications when no test is required.\n\t\t\t\t(Default)\n\n"
             <<"\t--save\t\t\tSave the current settings to the configuration\n\t\t\t\tfile. Users are not affected unless combined\n\t\t\t\twith --no-config.\n\n"
             <<"\t--single\t\tOnly tests for the defined user. Can only be\n\t\t\t\tused if a user is passed in the arguments.\n\n"
             <<"\t--sleep-time\t\tSet the number of seconds to wait between\n\t\t\t\tlooking up clients or retrying. (Default: "<<DEFAULT_SLEEP<<")\n\n"
             <<"\t--tries\t\t\tSet the number of times to attempt looking up\n\t\t\t\ta client. (Default: "<<DEFAULT_TRIES<<")\n\n"
             <<"\t--verbose\t\tAlways output test results to stdout.\n\n"
             <<"EXIT STATUS\n\tNormal exit status is 0. 1 if an error occurs with the configuration.\n\tEven if test results are invalid or a connection is not made the exit\n\tstatus is 0.\n"<<std::endl;
}

void usageInfo(char *cmd)
{
    std::cout<<"\nUSAGE\n\t"<<cmd<<" [OPTIONS] [<EMAIL> <PHONE_NUMBER> <LAST_NAME> <ID_NUMBER>]\n\tEMAIL can be '-' to send output to stdout.\n\tPHONE_NUMBER is the testing number of the agency you report to.\n"
             <<"\n\t\t'"<<cmd<<" --help' for more info.\n"<<std::endl;
}


