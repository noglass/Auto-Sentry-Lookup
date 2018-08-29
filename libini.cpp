#include <fstream>
#include <utility>
#include <stdexcept>
#include "libini.h"

INIObject::INISection::INISection(const std::string &topic, std::vector<INIObject::INIItem> &items)
{
    name = topic;
    data = std::move(items);
}

std::string& INIObject::INISection::operator() (const std::string &item, size_t skipItem)
{
    for (auto i = data.begin(), ie = data.end();i != ie;++i)
    {
        if (i->item == item)
        {
            if (!skipItem)
                return i->value;
            skipItem--;
        }
    }
    if (!skipItem)
    {
        data.push_back({item,""});
        return data.rbegin()->value;
    }
    throw std::out_of_range("INIObject::INISection::operator()");
}

std::string& INIObject::INISection::operator() (size_t item)
{
    return data.at(item).value;
}

std::string INIObject::INISection::find(const std::string &item, size_t skipItem)
{
    for (auto i = data.begin(), ie = data.end();i != ie;++i)
    {
        if (i->item == item)
        {
            if (!skipItem)
                return i->value;
            skipItem--;
        }
    }
    throw std::out_of_range("INIObject::INISection::find");
}

std::string INIObject::INISection::find(size_t item)
{
    return data.at(item).value;
}

std::string INIObject::INISection::info(size_t item)
{
    return data.at(item).item;
}

size_t INIObject::INISection::items(const std::string &item)
{
    size_t ret = 0;
    for (auto i = data.begin(), ie = data.end();i != ie;++i)
        if (i->item == item)
            ret++;
    return ret;
}

int INIObject::INISection::erase(const std::string &item, size_t skipItem)
{
    for (auto i = data.begin(), ie = data.end();i != ie;++i)
    {
        if (i->item == item)
        {
            if (!skipItem)
            {
                data.erase(i);
                return 0;
            }
            skipItem--;
        }
    }
    return 1;
}

int INIObject::INISection::erase(size_t item)
{
    data.erase(data.begin()+item);
    return 0;
}

std::vector<INIObject::INIItem>::iterator INIObject::INISection::erase(std::vector<INIObject::INIItem>::iterator it)
{
    return data.erase(it);
}

std::vector<INIObject::INIItem>::iterator INIObject::INISection::erase(std::vector<INIObject::INIItem>::iterator first, std::vector<INIObject::INIItem>::iterator last)
{
    return data.erase(first,last);
}


bool INIObject::INISection::exists(const std::string &item, size_t skipItem)
{
    return (items(item) > skipItem);
}

void noLead(std::string &str, const std::string &remove)
{
    for (auto it = str.begin();it != str.end();)
    {
        //if ((*it == ' ') || (*it == '\t'))
        if (remove.find(*it) != std::string::npos)
            it = str.erase(it);
        else
            break;
    }
}

void noTrail(std::string &str, const std::string &remove)
{
    if (str.size())
    {
        for (auto it = str.end()-1;it != str.begin();)
        {
            //if ((*it == ' ') || (*it == '\t'))
            if (remove.find(*it) != std::string::npos)
                it = str.erase(it)-1;
            else
                break;
        }
    }
}

void nospace(std::string &str)
{
    //for (auto pos = str.find("  ");pos != std::string::npos;pos = str.find("  "))
    //    str.erase(pos,1);
    std::string white = " \t";
    noLead(str,white);
    noTrail(str,white);
}

INIObject::INIObject(const std::string &filepath)
{
    open(filepath);
}

int INIObject::open(const std::string &filepath)
{
    data.clear();
    std::ifstream file (filepath);
    if (file.is_open())
    {
        std::vector<INIObject::INIItem> section;
        std::string topic;
        std::string line;
        std::string white = " \t";
        while (std::getline(file,line))
        {
            size_t l;
            nospace(line);
            if ((line.size() < 1) || (line.front() == '#'))
                continue;
            line = line.substr(0,line.find('\r'));
            if ((line.size() > 1) && (line.front() == '[') && (line.back() == ']'))
            {
                if ((topic.size() > 0) || (section.size() > 0))
                    data.push_back(INIObject::INISection(topic,section));
                section.clear();
                topic.assign(line,1,line.size()-2);
                nospace(topic);
            }
            else
            {
                std::string value = "";
                if ((l = line.find('=')) != std::string::npos)
                    value = line.substr(l+1);
                noLead(value,white);
                std::string item = line.substr(0,l);
                noTrail(item,white);
                section.push_back({item,value});
            }
        }
        if ((topic.size() > 0) || (section.size() > 0))
            data.push_back(INIObject::INISection(topic,section));
        file.close();
        return 0;
    }
    return 1;
}

int INIObject::write(const std::string &filepath)
{
    std::ofstream file (filepath,std::ofstream::out|std::ofstream::trunc);
    int ret = 0;
    if (file.is_open())
    {
        for (auto topic = data.begin(), topice = data.end();topic != topice;++topic)
        {
            file<<'['<<topic->topic()<<"]\n";
            for (auto item = topic->begin(), iteme = topic->end();item != iteme;++item)
                file<<item->item<<'='<<item->value<<'\n';
        }
        file.close();
    }
    else
        ret = 1;
    return ret;
}

std::string& INIObject::operator() (const std::string &topic, const std::string &item, size_t skipTopic, size_t skipItem)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return (*t)(item,skipItem);
            skipTopic--;
        }
    }
    if ((!skipTopic) && (!skipItem))
    {
        std::vector<INIObject::INIItem> temp;
        temp.push_back({item,""});
        data.push_back(INIObject::INISection(topic,temp));
        return data.rbegin()->rbegin()->value;
    }
    throw std::out_of_range("INIObject::operator()");
}

std::string& INIObject::operator() (const std::string &topic, size_t item, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return (*t)(item);
            skipTopic--;
        }
    }
    throw std::out_of_range("INIObject::operator()");
}

std::string& INIObject::operator() (size_t topic, const std::string &item, size_t skipItem)
{
    return data.at(topic)(item,skipItem);
}

std::string& INIObject::operator() (size_t topic, size_t item)
{
    return data.at(topic)(item);
}

std::vector<INIObject::INISection>::iterator INIObject::topic_it(const std::string &topic, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t;
            skipTopic--;
        }
    }
    throw std::out_of_range("INIObject::topic_it");
}

std::string INIObject::find(const std::string &topic, const std::string &item, size_t skipTopic, size_t skipItem)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t->find(item,skipItem);
            skipTopic--;
        }
    }
    throw std::out_of_range("INIObject::find");
}

std::string INIObject::find(const std::string &topic, size_t item, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t->find(item);
            skipTopic--;
        }
    }
    throw std::out_of_range("INIObject::find");
}

std::string INIObject::find(size_t topic, const std::string &item, size_t skipItem)
{
    return data.at(topic).find(item,skipItem);
}

std::string INIObject::find(size_t topic, size_t item)
{
    return data.at(topic).find(item);
}

std::string INIObject::info(size_t topic)
{
    return data.at(topic).topic();
}

std::string INIObject::info(size_t topic, size_t item)
{
    return data.at(topic).info(item);
}

std::string INIObject::info(const std::string &topic, size_t item, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t->info(item);
            skipTopic--;
        }
    }
    throw std::out_of_range("INIObject::info");
}

size_t INIObject::topics()
{
    return data.size();
}

size_t INIObject::topics(const std::string &topic)
{
    size_t ret = 0;
    for (auto t = data.begin(), te = data.end();t != te;++t)
        if (t->topic() == topic)
            ret++;
    return ret;
}

size_t INIObject::items(const std::string &topic, size_t skipTopic)
{
    size_t ret = 0;
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
            {
                ret = t->items();
                break;
            }
            skipTopic--;
        }
    }
    return ret;
}

size_t INIObject::items(size_t topic)
{
    return data.at(topic).items();
}

size_t INIObject::items(const std::string &topic, const std::string &item, size_t skipTopic)
{
    size_t ret = 0;
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
            {
                ret = t->items(item);
                break;
            }
            skipTopic--;
        }
    }
    return ret;
}

size_t INIObject::items(size_t topic, const std::string &item)
{
    return data.at(topic).items(item);
}

int INIObject::erase(const std::string &topic, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
            {
                data.erase(t);
                return 0;
            }
            skipTopic--;
        }
    }
    return 1;
}

int INIObject::erase(const std::string &topic, const std::string &item, size_t skipTopic, size_t skipItem)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t->erase(item,skipItem);
            skipTopic--;
        }
    }
    return 1;
}

int INIObject::erase(size_t topic)
{
    data.erase(data.begin()+topic);
    return 0;
}

int INIObject::erase(size_t topic, size_t item)
{
    return data.at(topic).erase(item);
}

int INIObject::erase(size_t topic, const std::string &item, size_t skipItem)
{
    return data.at(topic).erase(item,skipItem);
}

int INIObject::erase(const std::string &topic, size_t item, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t->erase(item);
            skipTopic--;
        }
    }
    return 1;
}

std::vector<INIObject::INISection>::iterator INIObject::erase(std::vector<INIObject::INISection>::iterator it)
{
    return data.erase(it);
}

std::vector<INIObject::INISection>::iterator INIObject::erase(std::vector<INIObject::INISection>::iterator first, std::vector<INIObject::INISection>::iterator last)
{
    return data.erase(first,last);
}

bool INIObject::exists(const std::string &topic, size_t skipTopic)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return true;
            skipTopic--;
        }
    }
    return false;
}

bool INIObject::exists(const std::string &topic, const std::string &item, size_t skipTopic, size_t skipItem)
{
    for (auto t = data.begin(), te = data.end();t != te;++t)
    {
        if (t->topic() == topic)
        {
            if (!skipTopic)
                return t->exists(item,skipItem);
            skipTopic--;
        }
    }
    return false;
}

bool INIObject::exists(size_t topic, const std::string &item, size_t skipItem)
{
    return data.at(topic).exists(item,skipItem);
}


