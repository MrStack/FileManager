#ifndef FILE_MANAGE
#define FILE_MANAGE

#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctime>
#include <dirent.h>
#include <cstdlib>
#include <vector>
#include <map>

using namespace std;

//File manage class.
//Listing files in a directory.
//Getting information of a file.

enum FilterMode{FM_YEARS,FM_MONTHS,FM_DAYS,FM_HOURS,FM_MINS,FM_SECS};

class File
{
private:
//Data members:
    DIR* m_pdirStream;
    dirent* m_pdirEnt;
    FILE* pFile;
    struct stat m_fileState;

    char** m_ppFileList;
    size_t m_FileListSize;

    char* m_pCurrentDir;
    size_t m_CurrentDirLen;

    map<char*,struct stat> m_FileInfo;//Contain the complete file path and the file state.
    vector<char*> m_FileFilted;//Contain the files were filted.

//Function members:
    void ListDir(void)
    {
        map<char*,struct stat>::iterator iter{begin(m_FileInfo)};
        for(size_t i{};i < m_FileListSize;i++)
        {
            if(S_ISDIR(iter->second.st_mode))
            {
// TODO (albertstack#1#): Make the strings be aligned
                cout<<"Dir:  "<<*(m_ppFileList + i)<<endl;
            }
            else
            {
                cout<<"      "<<*(m_ppFileList + i)<<endl;
            }
            iter++;
        }
    }

public:
    explicit File(void):m_pdirStream{},m_pdirEnt{},pFile{},m_fileState{},m_ppFileList{},m_FileListSize{},m_pCurrentDir{} {}
    ~File()
    {
        closedir(m_pdirStream);
        for(size_t i{};i < m_FileListSize;i++)
        {
            free(*(m_ppFileList + i));
        }
        free(m_ppFileList);
    }

    int ReadDir(const char* pPath)
    {
        if((m_pdirStream = opendir(pPath)) == NULL)
        {
            cout<<"open dir:"<<pPath<<" error"<<endl;
            return -1;
        }

        //Store the Path opend.
        m_CurrentDirLen = strlen(pPath);
        m_pCurrentDir = (char*)malloc(m_CurrentDirLen + 1);
        strncpy(m_pCurrentDir,pPath,m_CurrentDirLen);
        m_pCurrentDir[m_CurrentDirLen] = '\0';

        //List all the subdirectory and state the files.
        size_t i{};   //Index
        char* WorkingDir{nullptr};
        struct stat FileState{};
        size_t TempFileNameLen{};
        while((m_pdirEnt = readdir(m_pdirStream)) != NULL)
        {
            if(strcmp(m_pdirEnt->d_name,".") == 0 || strcmp(m_pdirEnt->d_name,"..") == 0) //Avoid the current and parent folder.
            {
                continue;
            }

            m_ppFileList = (char**)realloc(m_ppFileList,(i+1) * sizeof(char*));
            size_t NameLen{strlen(m_pdirEnt->d_name)};
            *(m_ppFileList + i) = (char*)malloc(NameLen * sizeof(char) + 1);  //Include the '\0' at the end of a string.
            strncpy(*(m_ppFileList + i),m_pdirEnt->d_name,NameLen);
            (*(m_ppFileList + i))[NameLen] = '\0';  //Add an end character.

            TempFileNameLen = strlen(*(m_ppFileList + i));
            WorkingDir = (char*)malloc(sizeof(char) * (m_CurrentDirLen + TempFileNameLen + 2));
            strncpy(WorkingDir,m_pCurrentDir,m_CurrentDirLen);
            WorkingDir[m_CurrentDirLen] = '/';
            strncpy(WorkingDir + m_CurrentDirLen + 1,*(m_ppFileList + i),TempFileNameLen);
            WorkingDir[m_CurrentDirLen + TempFileNameLen + 1] = '\0';
            stat(WorkingDir,&FileState);

            m_FileInfo.insert(make_pair(WorkingDir,FileState));
            m_FileListSize++;
            i++;
        }

        return 0;
    }

    void ListDir(const char* pCommand)
    {
        if(strcmp(pCommand,"") == 0)
        {
            for(size_t i{};i < m_FileListSize;i++)
            {
                if(*(m_ppFileList + i)[0] == '.')
                {
                    continue;
                }
                else
                {
                    cout<<*(m_ppFileList + i)<<endl;
                }
            }
        }
        else if(strcmp("-a",pCommand) == 0)
        {
            ListDir();
        }
    }

    int TimeFilter(enum FilterMode Mode,long StartTime,long EndTime)
    {//Usage:If put the start time 0, the end time must be negative.And the unit is always defined by the FilterMode.See case sentense below.
        struct tm* stmTime{(struct tm*)malloc(sizeof(struct tm))};
        time_t* pTime{(time_t*)malloc(sizeof(time_t))};
        if(StartTime == 0)
        {//Get the current time.
            if(EndTime > 0)
            {
                cout<<"Parameter fault"<<endl;
                return -1;
            }
            *pTime = time(nullptr);
            localtime_r(pTime,stmTime);
        }
        switch(Mode)
        {
            case FM_YEARS:
                stmTime->tm_year += EndTime;
                break;
            case FM_MONTHS:
                stmTime->tm_mon += EndTime;
                break;
            case FM_DAYS:
                stmTime->tm_mday += EndTime;
                break;
            case FM_HOURS:
                stmTime->tm_hour += EndTime;
                break;
            case FM_MINS:
                stmTime->tm_min += EndTime;
                break;
            case FM_SECS:
                stmTime->tm_sec += EndTime;
                break;
            default:
                cout<<"Parameter fault"<<endl;
                return -1;
        }
        *pTime = mktime(stmTime);

        for(map<char*,struct stat>::iterator iter{begin(m_FileInfo)};iter != end(m_FileInfo);iter++)
        {
            //Debug section
            struct tm* temp{(struct tm*)malloc(sizeof(struct tm))};
            time_t* pTimet{&(iter->second.st_atime)};
            localtime_r(pTimet,temp);
            cout<<iter->first<<endl;
            cout<<asctime(temp)<<endl;
            //Debug section
            if(*pTime - (iter->second.st_atime) >= 0)
            {
                m_FileFilted.push_back(iter->first);
            }
        }

        free(pTime);
        free(stmTime);

        //Debug section
        cout<<"These files were filted by the ";
        switch(Mode)
        {
            case FM_YEARS:
                cout<<"years";
                break;
            case FM_MONTHS:
                cout<<"months";
                break;
            case FM_DAYS:
                cout<<"days";
                break;
            case FM_HOURS:
                cout<<"hours";
                break;
            case FM_MINS:
                cout<<"minutes";
                break;
            case FM_SECS:
                cout<<"seconds";
                break;
        }
        cout<<" of start time and end time:"<<StartTime<<" to "<<EndTime<<endl;
        for(vector<char*>::iterator iter{begin(m_FileFilted)};iter != end(m_FileFilted);iter++)
        {
            cout<<*iter<<endl;
        }
        //Debug section

        return 0;
    }
};


#endif // FILE_MANAGE
