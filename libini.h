#ifndef _INI_VECTOR_
#define _INI_VECTOR_

#include <string>
#include <vector>

class INIObject
{
    private:
        struct INIItem
        {
            std::string item;
            std::string value;
        };
        
        class INISection
        {
            private:
                std::string name;
                std::vector<INIObject::INIItem> data;
            
            public:
                INISection() { data.clear(); }
                INISection(const std::string &topic, std::vector<INIObject::INIItem> &items);
                
                std::string& operator() (const std::string &item, size_t skipItem = 0);
                std::string& operator() (size_t item);
                
                std::vector<INIObject::INIItem>::iterator begin() { return data.begin(); }
                std::vector<INIObject::INIItem>::iterator end() { return data.end(); }
                std::vector<INIObject::INIItem>::reverse_iterator rbegin() { return data.rbegin(); }
                std::vector<INIObject::INIItem>::reverse_iterator rend() { return data.rend(); }
                std::vector<INIObject::INIItem>::const_iterator cbegin() { return data.cbegin(); }
                std::vector<INIObject::INIItem>::const_iterator cend() { return data.cend(); }
                std::vector<INIObject::INIItem>::const_reverse_iterator crbegin() { return data.crbegin(); }
                std::vector<INIObject::INIItem>::const_reverse_iterator crend() { return data.crend(); }
                
                std::string find(const std::string &item, size_t skipItem = 0);
                std::string find(size_t item);
                
                std::string topic() { return name; }
                
                std::string info(size_t item);
                
                size_t items() { return data.size(); }
                size_t items(const std::string &item);
                
                int erase(const std::string &item, size_t skipItem = 0);
                int erase(size_t item);
                std::vector<INIObject::INIItem>::iterator erase(std::vector<INIObject::INIItem>::iterator it);
                std::vector<INIObject::INIItem>::iterator erase(std::vector<INIObject::INIItem>::iterator first, std::vector<INIObject::INIItem>::iterator last);
                
                bool exists(const std::string &item, size_t skipItem = 0);
        };
        
        std::vector<INISection> data;
    
    public:
        INIObject() { data.clear(); }
        
        INIObject(const std::string &filepath);
        
        int open(const std::string &filepath);
        
        int write(const std::string &filepath);
        // writes the ini file
        
        std::string& operator() (const std::string &topic, const std::string &item, size_t skipTopic = 0, size_t skipItem = 0);
        // return reference to item
        
        std::string& operator() (const std::string &topic, size_t item, size_t skipTopic = 0);
        // return reference to item
        
        std::string& operator() (size_t topic, const std::string &item, size_t skipItem = 0);
        // return reference to item
        
        std::string& operator() (size_t topic, size_t item);
        // return reference to item
        
        std::vector<INIObject::INISection>::iterator begin() { return data.begin(); }
        std::vector<INIObject::INISection>::iterator end() { return data.end(); }
        std::vector<INIObject::INISection>::reverse_iterator rbegin() { return data.rbegin(); }
        std::vector<INIObject::INISection>::reverse_iterator rend() { return data.rend(); }
        std::vector<INIObject::INISection>::const_iterator cbegin() { return data.cbegin(); }
        std::vector<INIObject::INISection>::const_iterator cend() { return data.cend(); }
        std::vector<INIObject::INISection>::const_reverse_iterator crbegin() { return data.crbegin(); }
        std::vector<INIObject::INISection>::const_reverse_iterator crend() { return data.crend(); }
        
        std::vector<INIObject::INISection>::iterator topic_it(const std::string &topic, size_t skipTopic = 0);
        // returns an iterator pointing to the topic
        
        std::string find(const std::string &topic, const std::string &item, size_t skipTopic = 0, size_t skipItem = 0);
        
        std::string find(const std::string &topic, size_t item, size_t skipTopic = 0);
        
        std::string find(size_t topic, const std::string &item, size_t skipItem = 0);
        
        std::string find(size_t topic, size_t item);
        
        std::string info(size_t topic);
        // returns the name of the nth topic
        
        std::string info(size_t topic, size_t item);
        // returns the name of the nth item in the nth topic
        
        std::string info(const std::string &topic, size_t item, size_t skipTopic = 0);
        // returns the name of the nth item in the topic
        
        size_t topics();
        // returns the total number of topics
        
        size_t topics(const std::string &topic);
        // returns the total number of matching topics
        
        size_t items(const std::string &topic, size_t skipTopic = 0);
        // returns the total number of items in topic
        
        size_t items(size_t topic); // returns the total number of items in the topic
        
        size_t items(const std::string &topic, const std::string &item, size_t skipTopic = 0);
        // returns the total number of matching items in topic
        
        size_t items(size_t topic, const std::string &item);
        // returns the total number of matching items in topic
        
        int erase(const std::string &topic, size_t skipTopic = 0);
        // erases an entire section
        
        int erase(const std::string &topic, const std::string &item, size_t skipTopic = 0, size_t skipItem = 0);
        // erases an item from a section
        
        int erase(size_t topic);
        int erase(size_t topic, size_t item);
        int erase(size_t topic, const std::string &item, size_t skipItem = 0);
        int erase(const std::string &topic, size_t item, size_t skipTopic = 0);
        std::vector<INIObject::INISection>::iterator erase(std::vector<INIObject::INISection>::iterator it);
        std::vector<INIObject::INISection>::iterator erase(std::vector<INIObject::INISection>::iterator first, std::vector<INIObject::INISection>::iterator last);
        
        bool exists(const std::string &topic, size_t skipTopic = 0);
        bool exists(const std::string &topic, const std::string &item, size_t skipTopic = 0, size_t skipItem = 0);
        bool exists(size_t topic, const std::string &item, size_t skipItem = 0);
};

#endif

