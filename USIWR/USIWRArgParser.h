#ifndef USIWRARGPARSER_H
#define USIWRARGPARSER_H

#include <QString>
#include <map>
#include <vector>
using namespace std;

class USIWRArgParser
{
public:
    USIWRArgParser();
    USIWRArgParser(int argc, char *argv[]);
    ~USIWRArgParser();

    QString getStrValue(QString option);
    unsigned char* getByteArray(QString option);
    int getIntValue(QString option);

    bool isGUI();

    vector<QString> getCmdList() const;

    void doCommand();

private:
    map<QString,QString> m_optionValue;
    vector<QString> m_cmdList;
    vector<QString> m_cmdInfoList;

    bool m_isGUI;
};

#endif // USIWRARGPARSER_H
