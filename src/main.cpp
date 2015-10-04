#include <string>
#include <sstream>
#include <iostream>
#include <boost/regex.hpp>

//#include <fcgi_stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <mstch/mstch.hpp> 

#include "markdown.h"

int pagesServed = 0;

int initalize(void);
void printTree(xmlNodePtr inNode, uint nestLevel);
void mstchTest();
void markdownTest();

int main(int argc, char* argv[])
{    
    while (FCGI_Accept() >= 0) {
        print_header();
        print_body();
        print_close();
    }
    return 0;
}

void printTree(xmlNodePtr inNode, int nestLevel) {
    xmlNodePtr curNode;
    for (curNode = inNode; curNode; curNode = curNode->next) {
        if (curNode->type == XML_ELEMENT_NODE)
            printf("%0*s<%s>\n", nestLevel, " ", curNode->name); //get attributes
        if (curNode->type == XML_TEXT_NODE 
            && curNode->content
            && curNode->content[0] != '\n')
            printf("%0*s%s\n", nestLevel, " ", curNode->content);
        printTree(curNode->children, nestLevel + 1);
        if (curNode->type == XML_ELEMENT_NODE)
            printf("%0*s</%s>\n", nestLevel, " ", curNode->name);
    }
}

int initalize(void) {
    xmlDocPtr doc;
    xmlNodePtr node;
    
    doc = xmlParseFile("test.html");
    if (doc == NULL) {
        printf("Document failed to parse.\n");
        return 1;
    }
    node = xmlDocGetRootElement(doc);
    if (node == NULL) {
        printf("No root node found.\n");
        return 2;
    }
    printf("<%s>\n", node->name);
    printTree(node->children, 1);
    printf("</%s>\n", node->name);
    return 0;
}

void mstchTest() {
    std::string tmpl = "{{#names}}\nHi! {{name}}\n{{/names}}";
    mstch::map content {
      {"names", mstch::array{
          mstch::map{{"name", std::string("name 1")}},
          mstch::map{{"name", std::string("name 2")}},
          mstch::map{{"name", std::string("name 3")}},
          mstch::map{{"name", std::string("name 4")}}
      }}  
    };
    
    std::cout << mstch::render(tmpl, content) << std::endl;
    
}

void markdownTest() {
    std::string md = "# header 1\n## header 2\n some content\n*italic content* **and bold**\n * list item 1\n * item 2 \n * item 3\n";
    std::stringstream outStream;
    std::string out;
    markdown::Document doc;
    doc.read(md);
    
    doc.write(outStream);
    std::cout << md << std::endl;
    
    outStream >> out;
    do { 
        std::cout << out << " ";
        outStream >> out;
    } while (outStream.good());
    std::cout << std::endl;
}
