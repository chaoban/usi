#include "USIWRArgParser.h"
#include <QDebug>
#include <QThread>
#include <QStringList>
#include <QString>
#ifdef WIN32
#include "WinHIDFeatureCtrl.h"
#else
#include "LinuxHIDFeatureCtrl.h"
#endif

USIWRArgParser::USIWRArgParser()
    :m_isGUI(false)
{

}

USIWRArgParser::USIWRArgParser(int argc, char *argv[])
    :m_isGUI(false)
{
    QString str;
    for(int i = 1; i < argc ; i++)
    {
        str.append(argv[i]);
        str.append(" ");
    }
    QStringList strList = str.trimmed().split("-",QString::SkipEmptyParts);

    if(argc == 2)
    {
        if(strList[0] == "ui")
        {
            m_isGUI = true;
            qDebug()<<"Is GUI";
            return;
        }
    }

    for(int i = 0; i <strList.count(); i++)
    {
        QStringList option = strList[i].split("=",QString::SkipEmptyParts);
        m_optionValue[option[0]] = option[1].trimmed();

        m_cmdList.push_back(option[0]);
        m_cmdInfoList.push_back(strList[i].trimmed());
    }
}

USIWRArgParser::~USIWRArgParser()
{

}

QString USIWRArgParser::getStrValue(QString option)
{
    return m_optionValue[option];
}

unsigned char *USIWRArgParser::getByteArray(QString option)
{
    QStringList data = m_optionValue[option].split(" ",QString::SkipEmptyParts);
    unsigned char* ary = new unsigned char[data.count()];
    for(int i = 0; i < data.count();i++)
    {
        bool orz;
        ary[i] = data[i].toInt(&orz, 16);
    }
    return ary;
}

int USIWRArgParser::getIntValue(QString option)
{
    return m_optionValue[option].toInt();
}

bool USIWRArgParser::isGUI()
{
    return m_isGUI;
}

vector<QString> USIWRArgParser::getCmdList() const
{
    return m_cmdList;
}

void USIWRArgParser::doCommand()
{
    if(m_cmdList.size() == 0)
    {
        printf("Nothing to do.\n");
        return;
    }
    if(!SHIDFeatureCtrl.openDevice())
        return;

    for(int i = 0; i < m_cmdList.size();i++)
    {
        printf("do %s\n",m_cmdInfoList[i].toStdString().c_str());
        QString cmd = m_optionValue[m_cmdList[i]];
        printf("%s\n",cmd.toStdString().c_str());
        if(m_cmdList[i] == "w")
        {
            QStringList data = cmd.split(" ",QString::SkipEmptyParts);
            int length = data.count();
            unsigned char* ary = new unsigned char[length];
            for(int i = 0; i < length;i++)
            {
                bool orz;
                ary[i] = data[i].toInt(&orz, 16);
            }
            SHIDFeatureCtrl.setHIDFeature(ary, length);
        }
        else if(m_cmdList[i] == "delay")
        {
            int ms = cmd.toInt();
            QThread::msleep(ms);
        }
        else if(m_cmdList[i] == "r")
        {
            QStringList data = cmd.split(" ",QString::SkipEmptyParts);
            int length = data[1].toInt();
            unsigned char* ary = new unsigned char[length];
            bool orz;
            ary[0] = data[0].toInt(&orz, 16);
            SHIDFeatureCtrl.getHIDFeature(ary, length);
        }
    }
}
