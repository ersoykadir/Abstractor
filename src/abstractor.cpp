#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cstring>
#include <set>
#include <unordered_map>
#include <utility>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>
#include <semaphore.h>
using namespace std;



/**
 * @brief mutex to protect the access to file_index variable, each thread gets a file from the file list with the current index and increases it by one.
 */
pthread_mutex_t file_index_mutex = PTHREAD_MUTEX_INITIALIZER;
/**
 * @brief mutex to protect write back of calculated results to logs.
 */
pthread_mutex_t file_list_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief struct that holds the result info.
 */
struct log {
		//	Name of the abstract file.
		string abstract_name=" ";
		//	Jaccard similarity of this file with current query.
		double similarity=0;
		//	The sentence which got the most number of matching words with query.
		string summary=" ";
};

/**
 * @brief struct that holds the input data to threads.
 */
struct input_struct {
    //	The file that thread will process.
	string input_file_name= " ";
	//	The query of words to search in the file.
	string query = " ";
	//	The ID of the thread
	int threadID = -1;
	//	The sentence which got the most number of matching words with query for the current file that the thread is processing.
	string summary = " ";
	//	Jaccard similarity of the file ,which the thread is currently processing, with current query.
	double jaccard=0;
	//	The index of the file which the thread is currently processing.
	int current_file_index=-1;
	//	Indicates whether the thread is working or not.
	bool working = true;
};

//	Index of file name list.
int file_name_index=0;
//	The number of processes that wait on mutex lock.
int waiting_processes=0;
//	The list of output data log.
vector<log*> logs;
//	The output file name that the results will be written. 
string outfile_name;

/**
 * @brief Compares two log struct by their jaccard similarity scores.
 * 
 * @param a log struct to be compared.
 * @param b log struct to be compared.
 * @return true If similarity score of a is higher than b.
 * @return false otherwise.
 */
bool compareByLength(const log *a, const log *b)
{
    return a->similarity > b->similarity;
}

/**
 * make an abstracts list
 * make a threads list
 * have a counter
 * traverse abstracts
 * create thread with next element in abstractlist with counter%thread_num
 * if counter != 0 and counter % threadnum == 0
 * traverse thread list
 * 1) 	call join for all of them(??? should I wait for each one of them)
 * 2)	call join for thread_0 and || create thread_0 with next abstract || call join for next thread
 */

/**
 * @brief Tokenizes the given string with given seperator and fills it to the given list.
 * 
 * @param inputStr 			String to be tokenized.
 * @param tokens 			Gets filled by the tokens.
 * @param tokenSeperator 	Seperator character to tokenize the string.
 */
void tokenize(string inputStr, vector<string>* tokens, char tokenSeperator);

/**
 * @brief Returns the number of intersecting words between two sets.
 * 
 * @param query_set 		Set of query words to look for.
 * @param abstract_set 		Set of words to search.
 * @param intersection_set 	Intersecting set of words in query and the file
 * @return int 				Number of intersecting words.
 */
int similarity(set<string> query_set, set<string> sentence_set, set<string> *intersection_set);

/**
 * @brief Calculating the jaccard similarity score of the input.
 * 
 * @param inputArgs Struct that holds necessary input for the thread.
 */
void jaccard(input_struct *inputArgs);

/**
 * @brief Runner function that threads execute.
 */
void *runner(void*);


int main(int argc, char* argv[]) 
{
	string infile_name = argv[1];//READ INPUT FILE NAME
	outfile_name = argv[2];//READ OUTPUT FILE NAME

	//OPEN INPUT AND OUTPUT FILE STREAMS
	ifstream infile;
	infile.open(infile_name);
	ofstream outfile;
	//	Clear the output file before appending...
	outfile.open(outfile_name);
	outfile.close();
	//	...
	outfile.open(outfile_name, std::ios::out | std::ios::app );

	vector<string> lines;//lines of the file being read
	string line;
	while(getline(infile, line)){
		lines.push_back(line);
	}
	/*	First line holds the information about number of threads, number of files to read and number of results to return. */
    string info = lines[0];
	//---	Reading the numbers into integers.
    stringstream ss(info);
    string thread_numStr;
    string num_abstr_to_scanStr;
    string num_abstr_to_returnStr;
	ss >> thread_numStr;
	ss >> num_abstr_to_scanStr;
	ss >> num_abstr_to_returnStr;
	if(thread_numStr.empty() || num_abstr_to_scanStr.empty() || num_abstr_to_returnStr.empty()){
        cout << "Parameters missing!!!" << endl;
        return(1);
   	}
	int thread_num= stoi(thread_numStr);//						number of threads.
	int num_abstr_to_scan = stoi(num_abstr_to_scanStr);//		number of abstract files.
	int num_abstr_to_return = stoi(num_abstr_to_returnStr);//	number of results to return.
	//---
   	string query = lines[1];//	Query words to search within files.
	   
	//Each file has its own log including summary and jaccard similarity score.
	//Initializing logs.
   	for (int i = 2; i < lines.size(); ++i)
   	{
		logs.push_back(new log);
		logs[i-2]->abstract_name = lines[i];
   	}

	vector<pthread_t> threads;
	waiting_processes = thread_num;// number of processes that are waiting for a file to process.
	vector<input_struct*>inputs;

   	// Create threads.
	for (int i = 0; i < thread_num; i++)
	{
		pthread_t tid;
		threads.push_back(tid);
		inputs.push_back(new input_struct);
		inputs[i]->threadID = i;
		inputs[i]->query = query;
		pthread_create(&threads[i],NULL,runner,inputs[i]);
	}
	//	Wait for each thread to join back to main.
	for (int i = 0; i < thread_num; i++)
		pthread_join(threads[i], NULL);
	
	//	Sort the logs w.r.t similarity scores.
	sort(logs.begin(), logs.end(), compareByLength);

	//	Print out the results to the output file.
	outfile << "###" << endl;
	for (int i = 0; i < num_abstr_to_return; i++)
	{
		outfile << "Result "<< i+1 << ":" << endl;
		outfile << "File: " << logs[i]->abstract_name << endl;
		outfile << setprecision(4) << fixed << "Score: " << logs[i]->similarity << endl;
		outfile << "Summary: " << logs[i]->summary << endl;	
		outfile << "###" << endl;
	}

	//	Cleaning up.
	for (int i = 0; i < logs.size(); i++)
	{
		delete logs[i];
	}
	for (int i = 0; i < inputs.size(); i++)
	{
		delete inputs[i];
	}
	infile.close();
	outfile.close();
}

void *runner(void *arguments) 
{
	struct input_struct *args = (struct input_struct *)arguments;
	do
	{
		/*	Request file list access and get a file to process. */
		pthread_mutex_lock(&file_index_mutex);

		if(file_name_index==logs.size())//Every existing file is acquired, exit the thread.
		{
			args->working = false;
			waiting_processes--;
			pthread_mutex_unlock(&file_index_mutex);
    		pthread_exit(NULL);		
		}
		else//	There are still some files to be processed.
		{
			/*	Acquire a file with the current index. */ 
			args->input_file_name = logs[file_name_index]->abstract_name;
			args->current_file_index = file_name_index;
			//	Increase the index for the next thread.
			file_name_index++;
			//	Write to output, informing that this thread is processing the taken file.
			ofstream outfile;
			outfile.open(outfile_name, std::ios::app);
			outfile << "Thread " << char(65 + args->threadID) << " is calculating " << args->input_file_name << endl;
			outfile.close();
			//--
			waiting_processes--;//	Decrease the number of waiting processes since this thread acquired a file.
		}

		pthread_mutex_unlock(&file_index_mutex);//	File acquired, will be processed.

		/*	Starting calculation. */
		jaccard(args);

		/*	Request file list access to log the results. */
		pthread_mutex_lock(&file_list_mutex);
		logs[args->current_file_index]->similarity = args->jaccard;
		logs[args->current_file_index]->summary = args->summary;
		pthread_mutex_unlock(&file_list_mutex);

		while(waiting_processes > 0){}//The thread will not proceed to get a new file if there already is a thread waiting.
		
		pthread_mutex_lock(&file_index_mutex);//Checking the exit conditions
		if(file_name_index == logs.size())	
			args->working = false;
		else
			waiting_processes++;
		pthread_mutex_unlock(&file_index_mutex);
		
	} while(args->working);
	
    pthread_exit(NULL);
}

void jaccard(input_struct *inputArgs)
{
	//	Reading the given input file.
	ifstream infile;
	string file_path = "../abstracts/" + inputArgs->input_file_name;
	infile.open(file_path);
	string abstract;
	string line;
	while(getline(infile, line)){
		abstract += line;
	}

	string query = inputArgs->query;
	vector<string> abstract_vector;
	vector <string> query_vector;
	
	//	Get query set.
	tokenize(query,&query_vector,' ');
	set<string> query_set(query_vector.begin(), query_vector.end());
	//	Get sentences.
	tokenize(abstract,&abstract_vector,'.');

	//	Search the file to find if it includes the query words.
	set<string> intersection_set ;
	int best_match = 0;
	string best_sentence="";//	Summary sentence.
	set<string> union_set = query_set;
	for (int i = 0; i < abstract_vector.size(); ++i)//Traverse each sentence
	{
		//	Tokenize each sentence to seperate words.
		vector<string> sentence;
		tokenize(abstract_vector[i],&sentence,' ');
		set<string> sentence_set(sentence.begin(), sentence.end());
		int matching_words = 0;
		matching_words = similarity(query_set,sentence_set,&intersection_set);//	Calculate the number of intersecting words.
		//	Check if the current sentence is the best up until here.
		if(matching_words > best_match){
			best_match = matching_words;
			inputArgs->summary = abstract_vector[i] + ".";// Update the summary.
		}
		for(auto w : sentence){
			union_set.insert(w);//	Insert the words in the current sentence to the total union.
		}
	}
	union_set.insert(".");
	double jaccard = double(intersection_set.size()) / union_set.size();
	inputArgs->jaccard = jaccard;
}

void tokenize(string inputStr, vector<string>* tokens, char tokenSeperator)
{
    stringstream check1(inputStr);
    string token;
    // Tokenizing w.r.t. given seperator.
    while(getline(check1, token, tokenSeperator))
    {
    	string sstemp = token;	
		stringstream ss(sstemp);
    	string word1;
   		ss >> word1;
   		if(!word1.empty())
        	tokens->push_back(token);
    }
}

int similarity(set<string> query_set, set<string> sentence_set, set<string> *intersection_set){

	int intersectionOld = intersection_set->size();//	Number of intersecting words before current sentence
	//	Traverse the current sentence and find the intersecting words.
	for (auto word : query_set)
	{
		if(sentence_set.count(word)){
			intersection_set->insert(word);
		}
	}
	//	Return the number of intersecting words by looking at how much the intersection set size increased.
	return intersection_set->size()-intersectionOld;
}