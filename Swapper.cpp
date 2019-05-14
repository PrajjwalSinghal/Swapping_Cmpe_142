/*
 • ‘C’ means the process is created (‘PAGE’ field not present)
 • ‘T’ means the process terminated (‘PAGE’ field not present)
 • ‘A’ means the process allocated memory at address ‘PAGE’
 • ‘R’ means the process read ‘PAGE’
 • ‘W’ means the process wrote to ‘PAGE’
 • ‘F’ means the process freed memory at address ‘PAGE’
 */
#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<unordered_map>
#include<tuple>
#include <random>
#include <algorithm>

using namespace std;

//***** Setting a limit to the Max number of Physical Pages ***//
// Change it to 20 before submitting
const int PhysicalPageLimit = 20;
const int VirtualPageLimit = 20;
//*************************************************************//
string algorithm = "";
vector <int> LRUqueue;
vector <int> FIFOqueue;
//*************************************************************//
//*** Function to run all the algorithm ***//
void RunSwapper(string input_file_path);

//*** Funtion to parse the input line into 3 separate parts ***//
tuple<int, char, int> parse(string ProcessEntry);

//*** Function to run a single command from input file ***//
void RunCommand(tuple<int, char, int> &Command,
                unordered_map<int, vector<int>> &ProcessTable,
                vector<tuple<int, int>> &PhysicalPages);
int getFreePage(unordered_map<int, vector<int>> &ProcessTable,
                vector<tuple<int, int>> &PhysicalPages);
void PrintMemoryDetails(unordered_map<int, vector<int>> &ProcessTable,
                        vector<tuple<int, int>> &PhysicalPages);
int getLRUPageIndex();
void updateLRUPageIndex(int PhysicalPageNo);
void removeLRUPageIndex(int PhysicalPageNo);

int getFIFOPageIndex();
void updateFIFOPageIndex(int PhysicalPageNo);
void removeFIFOPageIndex(int PhysicalPageNo);
// ******************************************************** //

//*** Functions to perform Different Possible Actions ***//
void Create(tuple<int, char, int> &Command,
            unordered_map<int, vector<int>> &ProcessTable,
            vector<tuple<int, int>> &PhysicalPages);
void Terminate(tuple<int, char, int> &Command,
               unordered_map<int, vector<int>> &ProcessTable,
               vector<tuple<int, int>> &PhysicalPages);
void Allocate(tuple<int, char, int> &Command,
              unordered_map<int, vector<int>> &ProcessTable,
              vector<tuple<int, int>> &PhysicalPages);
void Read(tuple<int, char, int> &Command,
          unordered_map<int, vector<int>> &ProcessTable,
          vector<tuple<int, int>> &PhysicalPages);
void Write(tuple<int, char, int> &Command,
           unordered_map<int, vector<int>> &ProcessTable,
           vector<tuple<int, int>> &PhysicalPages);
void Free(tuple<int, char, int> &Command,
          unordered_map<int, vector<int>> &ProcessTable,
          vector<tuple<int, int>> &PhysicalPages);
// ******************************************************** //

//*** Main Funtion ***//
int main(int argc, char** argv)
{
    // *** Takes the arguments the same way as the we did Scheduler Assignment ***//
    if(argc != 2){
        cout << "Wrong number of Input Files: " << endl;
        exit(1);
    }
    string inputFileName = argv[1];     // memory.dat
    string input_file_path = inputFileName;        // argv[1] = Input file name
    cout << "Output of LRU::" << endl;
    algorithm = "LRU";
    RunSwapper(input_file_path);

    cout << "\nOutput of FIFO::" << endl;
    algorithm = "FIFO";
    RunSwapper(input_file_path);

    cout << "\nOutput of Random::" << endl;
    algorithm = "Random";
    RunSwapper(input_file_path);
    //***********************************//
    return 0;
}
void RunSwapper(string input_file_path)
{
    ifstream in;
    in.open(input_file_path.c_str());
    
    if(in.fail()){
        cout << "Error in Opening Files"<<endl;
        exit(1);
    }
    // Declare a Physical Memory with 20 Pages
    vector<tuple<int,int>> PhysicalPages(PhysicalPageLimit, make_pair(-1,0));   // 0 : Free, 1 : Unmodified, 2 : Modified

    // Datastructure to Store Process ID and its details
    //          PROCESS_ID         Phys Pg No.
    unordered_map<int, vector<int>> ProcessTable;
    
    // String to read a single line input
    string ProcessEntry;
    
    //Skip the heading
    getline(in, ProcessEntry);
    
    // Keep Reading till the end of File
    while(!in.eof()) {
        // Read 1 line at a time
        getline(in, ProcessEntry);

        // Parse and store the command as (PROCESS_ID, ACTION, PAGE_NO)
        tuple<int, char, int> Command = parse(ProcessEntry);   // if PAGE_NO == -1, means no page was passed

        // The last command was coming out as empty
        if (get<1>(Command) == '\0')
            continue;

        RunCommand(Command, ProcessTable, PhysicalPages);
    }

    PrintMemoryDetails(ProcessTable, PhysicalPages);

    in.close();
    
}
tuple<int, char, int> parse(string ProcessEntry)
{
    int ProcessID;
    char Action;
    int PageNo = -1;
    int flag = 0;
    string temp = "";
    for(int i = 0; i < ProcessEntry.size(); i++)
    {
        if(flag == 0)
        {
            if(ProcessEntry[i] != ' ' && ProcessEntry[i] != '\t' && ProcessEntry[i] != '\n')
                temp += ProcessEntry[i];
            else{
                ProcessID = stoi(temp);
                flag = 1;
                temp.clear();
            }
        }
        else if(flag == 1){
            if(ProcessEntry[i] != ' ' && ProcessEntry[i] != '\t')
            {
                Action = ProcessEntry[i];
                flag = 2;
            }
        }
        
        else if(flag == 2){
            if(ProcessEntry[i] != ' ' && ProcessEntry[i] != '\t')
                temp += ProcessEntry[i];
        }
    }
    if(temp.empty() == 0)
        PageNo = stoi(temp);
    tuple<int, char, int> t1(ProcessID, Action, PageNo);
    return t1;
}
void RunCommand(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
                vector<tuple<int, int>> &PhysicalPages)
{
    switch (get<1>(Command)) {
        case 'C':
            Create(Command, ProcessTable, PhysicalPages);
            break;
        case 'T':
            Terminate(Command, ProcessTable, PhysicalPages);
            break;
        case 'A':
            Allocate(Command, ProcessTable, PhysicalPages);
            break;
        case 'R':
            Read(Command, ProcessTable, PhysicalPages);
            break;
        case 'W':
            Write(Command, ProcessTable, PhysicalPages);
            break;
        case 'F':
            Free(Command, ProcessTable, PhysicalPages);
            break;
        default:
            break;
    }
}
void Create(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
            vector<tuple<int, int>> &PhysicalPages)
{
    //Create a New entry in the Process Table
    vector<int> emptyPageTable(VirtualPageLimit, -2);         // -2 : No page was assigned, -1 : Page was assigned but was swapped
    ProcessTable[get<0>(Command)] = emptyPageTable;
    return;
}
void Terminate(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
               vector<tuple<int, int>> &PhysicalPages)
{
    // Remove the pages from the PhysicalPages if any

    auto ProcessID = ProcessTable.find(get<0>(Command));    // get the PageTable of the Process
    if(ProcessID == ProcessTable.end())                     // If the Process does not exist
        return;

    // Iterate through the Virtual pages of the Process
    for(auto itr : ProcessID->second)
        if(itr != -1 && itr != -2 )                                       // -1 meant that no physical page was assigned to that particular Virtual Page
        {
            if(algorithm == "LRU")
                removeLRUPageIndex(itr);                        // Remove from LRUqueue
            if(algorithm == "FIFO")
                removeFIFOPageIndex(itr);                       // Remove from FIFO queue
            get<1>(PhysicalPages[itr]) = 0;                 //  0 means the page is now free
            get<0>(PhysicalPages[itr]) = -1;                //  -1 means that no process owns that page
        }


    ProcessTable.erase(ProcessID->first);                   // Remove the process from Process Table(HashMap that contains all the process tables)
    return;
}
void Allocate(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
              vector<tuple<int, int>> &PhysicalPages)
{
    auto ProcessID = ProcessTable.find(get<0>(Command));
    if(ProcessID == ProcessTable.end())
        return;

    // FreePage is the index on physical pages that is now free
    int FreePage = getFreePage(ProcessTable,PhysicalPages);
    //auto EmptyPhyPN = find_if((ProcessID->second).begin(), (ProcessID->second).end(), [](const int& a) {return a == -2;});  // Find an unassigned page (-2 means the page wasn't assigned)
    //*EmptyPhyPN = FreePage;
    // auto VirtualPageTable = ProcessID->second;
    if(algorithm == "LRU")
        updateLRUPageIndex(FreePage);
    if(algorithm == "FIFO")
        updateFIFOPageIndex(FreePage);
    ProcessID->second[get<2>(Command)] = FreePage;
    get<1>(PhysicalPages[FreePage]) = 1;                    // Means the page is allocates and unmodified
    get<0>(PhysicalPages[FreePage]) = get<0>(Command);      // Process ID of the process that owns it

    return;
}
void Read(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
          vector<tuple<int, int>> &PhysicalPages)
{
    // Page number starts from 0
    auto ProcessID = ProcessTable.find(get<0>(Command));
    if (ProcessID == ProcessTable.end())
        return;
    auto ProcessVirtualTable = ProcessID->second;

    // Case1: Process has never assigned the page, likely to get terminated
    if(ProcessVirtualTable[get<2>(Command)] == -2) {
        tuple<int, char, int> TerminateCommand(get<0>(Command), 'T', -1);
        Terminate(TerminateCommand, ProcessTable, PhysicalPages);
        cout << "PROCESS " << get<0>(Command) << "\t" << "KILLED" << endl;
        return;
    }

    // Case2: Process has assigned the page but it is swapped out
    else if(ProcessVirtualTable[get<2>(Command)] == -1)
    {
        // in case of LRU the page is Updated in Allocate
        tuple<int, char, int> AllocateCommand(get<0>(Command), 'A', get<2>(Command));
        Allocate(AllocateCommand, ProcessTable, PhysicalPages);
        return;
    }

    // Case3: The process was already assigned and allocated that page
    if(algorithm == "LRU")
    {
        int PhysicalPageIndex = ProcessVirtualTable[get<2>(Command)];
        updateLRUPageIndex(PhysicalPageIndex);
    }

    return;
}
void Write(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
           vector<tuple<int, int>> &PhysicalPages)
{
    auto ProcessID = ProcessTable.find(get<0>(Command));
    if (ProcessID == ProcessTable.end())
        return;
    auto ProcessVirtualTable = ProcessID->second;

    // Case1: Process has never assigned the page, likely to get terminated
    if(ProcessVirtualTable[get<2>(Command)] == -2) {
        tuple<int, char, int> TerminateCommand(get<0>(Command), 'T', -1);
        Terminate(TerminateCommand, ProcessTable, PhysicalPages);
        cout << "PROCESS " << get<0>(Command) << "\t" << "KILLED" << endl;
        return;
    }

    // Case2: Process has assigned the page but it is swapped out
    else if(ProcessVirtualTable[get<2>(Command)] == -1)
    {
        // in case of LRU the page is Updated in Allocate
        tuple<int, char, int> AllocateCommand(get<0>(Command), 'A', get<2>(Command));
        Allocate(AllocateCommand, ProcessTable, PhysicalPages);
        // We have to change the status as modified
        int PhyscalPageIndex = ProcessVirtualTable[get<2>(Command)];
        get<1>(PhysicalPages[PhyscalPageIndex]) = 2;
        return;
    }
    // Case3: The process was already assigned and allocated that page
    else{
        int PhysicalPageIndex = ProcessVirtualTable[get<2>(Command)];
        get<1>(PhysicalPages[PhysicalPageIndex]) = 2;
        if(algorithm == "LRU")
            updateLRUPageIndex(PhysicalPageIndex);
        return;
    }
}
void Free(tuple<int, char, int> &Command, unordered_map<int, vector<int>> &ProcessTable,
          vector<tuple<int, int>> &PhysicalPages)
{
    auto ProcessID = ProcessTable.find(get<0>(Command));
    if (ProcessID == ProcessTable.end())
        return;
    auto ProcessVirtualTable = ProcessID->second;
    // Case1: The page was never Assigned, Terminate
    if(ProcessVirtualTable[get<2>(Command)] == -2)
    {
        tuple<int, char, int> TerminateCommand(get<0>(Command), 'T', -1);
        Terminate(TerminateCommand, ProcessTable, PhysicalPages);
        cout << "PROCESS " << get<0>(Command) << "\t" << "KILLED" << endl;
    }
    // Case2: The page was assigned but swapped out
    else if(ProcessVirtualTable[get<2>(Command)] == -1)
    {
        ProcessID->second[get<2>(Command)] = -2;
    }
    // Case3: The process owns that page
    else
    {
        int PhysicalPageIndex = ProcessVirtualTable[get<2>(Command)];
        if(algorithm == "LRU")
            removeLRUPageIndex(PhysicalPageIndex);
        if(algorithm == "FIFO")
            removeFIFOPageIndex(PhysicalPageIndex);

        ProcessID->second[get<2>(Command)] = -2;        // Marks as not owned
        get<0>(PhysicalPages[PhysicalPageIndex]) = -1;
        get<1>(PhysicalPages[PhysicalPageIndex]) = 0;
    }
    return;
}
int getFreePage(unordered_map<int, vector<int>> &ProcessTable,
                vector<tuple<int, int>> &PhysicalPages)
{
    // Search for a free page
    auto FreePagePosition = find_if(PhysicalPages.begin(), PhysicalPages.end(), [](const tuple<int,int>& a) {return get<1>(a) == 0;});  // find if the there is a page with value 0 in Physical Page table
    if(FreePagePosition != PhysicalPages.end())                     // If a page was found
        return distance(PhysicalPages.begin(), FreePagePosition);   // Return the index of that element

    // Search for Unmodified Page
    auto UnmodifiedPagePosition = find_if(PhysicalPages.begin(), PhysicalPages.end(), [](const tuple<int,int>& a) {return get<1>(a) == 1;}); // If the val is 1 (Unmodified Page)
    if(UnmodifiedPagePosition != PhysicalPages.end())
    {
        int Index = distance(PhysicalPages.begin(), UnmodifiedPagePosition);        // Get the index of that page
        int ProcessID = get<0>(*UnmodifiedPagePosition);                            // The process ID is stored in physical pages array
        auto SwapID = ProcessTable.find(ProcessID);                                 // Find the iterator to the Process Table HashMap
        for(int i = 0; i < SwapID->second.size(); i++)                                              // Iterate through the Virtual Page table of the process
        {
            if(SwapID->second[i] == Index)                                                        // When the page is found
            {
                SwapID->second[i] = -1;                                                           // Make VPT entry as -1 (Showing that it was swapped)
                get<1>(PhysicalPages[Index]) = 0;                                   // Free the page
                get<0>(PhysicalPages[Index]) = -1;                                  // No process owns it now
                return Index;                                                       // Return the nidex of the Physical Page
            }
        }
    }
    // Swap a Modified Page                                                         // When no free or unmodified pages are present


    auto FirstModifiedPage = PhysicalPages.begin();

    if(algorithm == "FIFO")
    {
        int increment = getFIFOPageIndex();
        FirstModifiedPage = PhysicalPages.begin() + increment;
    }
    else if(algorithm == "Random")
    {
        random_device rand_dev;
        mt19937 generator(rand_dev());
        uniform_int_distribution<int> range(0,PhysicalPageLimit-1);
        int increment = range(generator);
        FirstModifiedPage = PhysicalPages.begin() + increment;
    }
    else if(algorithm == "LRU")
    {
        int increment = getLRUPageIndex();
        FirstModifiedPage = PhysicalPages.begin() + increment;
    }
    if(FirstModifiedPage != PhysicalPages.end())
    {
        int Index = distance(PhysicalPages.begin(), FirstModifiedPage);             // Get the index of First Page
        int ProcessID = get<0>(*FirstModifiedPage);                                 // Get the Process ID
        auto SwapID = ProcessTable.find(ProcessID);                                 // Search for the Page Table of that process ID
        for(int i = 0; i < SwapID->second.size(); i++)
        {
            if(SwapID->second[i] == Index)
            {
                SwapID->second[i] = -1;                                             // -1 means that the page was swapped
                get<1>(PhysicalPages[Index]) = 0;                                   // free the page
                get<0>(PhysicalPages[Index]) = -1;                                  // No process owns it now
                return Index;                                                       // return the index to that page
            }
        }
    }
}

int getLRUPageIndex()
{
    //1. get the value (Page index) of the first element
    //2. Pop it from the first plance and put it at the end
    int PageIndex = LRUqueue[0];
    LRUqueue.erase(LRUqueue.begin());
    LRUqueue.push_back(PageIndex);

    return PageIndex;
}
void updateLRUPageIndex(int PhysicalPageNo)
{
    //1. Find the page in the LRUqueue
    //2. if it exists then remove it from its position and push it back at the end of the vector
    auto itr = find(LRUqueue.begin(), LRUqueue.end(), PhysicalPageNo);
    if (itr != LRUqueue.end())
        LRUqueue.erase(itr);
    LRUqueue.push_back(PhysicalPageNo);
}
void removeLRUPageIndex(int PhysicalPageNo)
{
    //1. Find the page in LRUqueue
    auto itr = find(LRUqueue.begin(), LRUqueue.end(), PhysicalPageNo);
    if (itr != LRUqueue.end())
        LRUqueue.erase(itr);
}
int getFIFOPageIndex()
{
    int PageIndex = FIFOqueue.back();
    FIFOqueue.pop_back();
    FIFOqueue.insert(FIFOqueue.begin(), PageIndex);

    return PageIndex;
}
void updateFIFOPageIndex(int PhysicalPageNo)
{
    //1. Find the page in the FIFOqueue
    //2. if it exists do nothing remove it from its position and push it back at the end of the vector
    auto itr = find(FIFOqueue.begin(), FIFOqueue.end(), PhysicalPageNo);
    if (itr == FIFOqueue.end())
        FIFOqueue.insert(FIFOqueue.begin(),PhysicalPageNo);
}
void removeFIFOPageIndex(int PhysicalPageNo)
{
    //1. Find the page in FIFOqueue
    auto itr = find(FIFOqueue.begin(), FIFOqueue.end(), PhysicalPageNo);
    if (itr != FIFOqueue.end())
        FIFOqueue.erase(itr);
}
void PrintMemoryDetails(unordered_map<int, vector<int>> &ProcessTable,
                        vector<tuple<int, int>> &PhysicalPages) {
    // Print Process Details
    for (auto Pitr : ProcessTable) {
        cout << "PROCESS" << "\t" << Pitr.first << endl;
        for (int i = 0; i < Pitr.second.size(); i++) {
            if (Pitr.second[i] == -1) {
                cout << "VIRTUAL" << "\t" << i << "\t" << "PHYSICAL" << "\t" << "SWAPPED" << endl;
            } else if (Pitr.second[i] != -2) {
                cout << "VIRTUAL" << "\t" << i << "\t" << "PHYSICAL" << "\t" << Pitr.second[i] << endl;
            }
        }
    }

    // Print Physical Page Status
    cout << "PHYSICAL" << endl;
    for (int i = 0; i < PhysicalPages.size(); i++)
    {
        if(get<0>(PhysicalPages[i]) == -1)
        {
            cout << i << "\t" << "FREE" << endl;
        }
        else{
            cout << i << "\t" << "PROCESS" << "\t" << get<0>(PhysicalPages[i]) << endl;
        }

    }

    return;
}
