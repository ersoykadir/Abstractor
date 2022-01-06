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

//double similarity(vector<string> query_set, unordered_map<vector<string>*,int> *abstract_set);
void tokenize(string inputStr, vector<string>* tokens, char tokenSeperator);
//void getSentences(string abstract, unordered_map<vector<string>*, int>* abstract_set);

//int similarity(set<string> query_set, set<string> abstract_set);
int similarity(set<string> query_set, set<string> abstract_set, set<string> *intersection_set);
//double similarity(set<string> query_set, set<string> abstract_set);
void tokenize(string inputStr, set<string>* tokens, char tokenSeperator);
void getSentences(string abstract, unordered_map<set<string>*, int>* abstract_set);

pthread_mutex_t file_index_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;

struct log {
	// have a list of logs, each thread will modify 
	//	the appropriate log in the list at the end of calculation
	// use mutex to modify the logs
		double similarity=0;
		string abstract_name=" ";
		string summary=" ";
};
struct input_struct {
    //string query;
	string input_file_name= " ";
	string query = " ";
	int threadID = -1;
	string summary = " ";
	double jaccard=0;
	int current_file_index=-1;
	bool work = true;
};


// struct input_struct {
// 	struct output_struct {
// 		double similarity=0;
// 		string abstract_name="";
// 		string summary="";
// 	};
//     //string input_file_name;
// 	string output_file_name;
//     string query;
// 	vector<string> *file_names;
// 	int threadID;
// 	char threadName = 'A';
// 	output_struct output ;
// };

int file_name_index=0;
int waiting_processes=0;
vector<log*> logs;
string outfile_name;

void jaccard(input_struct *inputArgs);
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

void *runner(void*); /* threads call this function */

int main(int argc, char* argv[]) {

	string infile_name = argv[1];//READ INPUT FILE NAME
	outfile_name = argv[2];//READ OUTPUT FILE NAME

	//OPEN INPUT AND OUTPUT FILE STREAMS
	ifstream infile;
	infile.open(infile_name);
	ofstream outfile;
	outfile.open(outfile_name);
	outfile.close();
	outfile.open(outfile_name, std::ios::out | std::ios::app );
	//cout << outfile_name << endl; 
	// //SEMAPHORE
	// sem_t sem;
	// if(sem_init(&sem,0,1) == -1)
	// 	printf("%s\n",strerror(errno));
	
	// /*	sem_wait decrements the semaphore value. Returns zero on success and -1 on error*/
	// if(sem_wait(&sem) != 0)
	// 	printf("%s\n",strerror(errno));
	
	// //Critical section ....
	// /*	sem_post increments the semaphore. */
	// if(sem_post(&sem) != 0)
	// 	printf("%s\n",strerror(errno));
	// /*	sem_destroy destroys the semaphore. */
	// if(sem_destroy(&sem) != 0)
	// 	printf("%s\n",strerror(errno));

	vector<string> lines;
	//vector<string> file_names;
	string line;
	while(getline(infile, line)){
		lines.push_back(line);
	}
    string info = lines[0];
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
	int thread_num= stoi(thread_numStr);
	int num_abstr_to_scan = stoi(num_abstr_to_scanStr);
	int num_abstr_to_return = stoi(num_abstr_to_returnStr);
   	string query = lines[1];
   	for (int i = 2; i < lines.size(); ++i)
   	{
		logs.push_back(new log);
		logs[i-2]->abstract_name = lines[i];//!!!!!!!!!
   		//file_names.push_back(lines[i]);
   	}  
	   cout << query << endl; 
	// Disable line buffering in stdout so that output gets displayed immediately
    //setvbuf(stdout, NULL, _IONBF, 0);
   	// pthread_t tid1,tid2; /* thread identifiers */
    // pthread_attr_t attr; /* set of thread attributes */
   	/* create threads */
	vector<pthread_t> threads;
	waiting_processes = thread_num;
	cout << query << endl; 
	vector<input_struct*>inputs;
	for (int i = 0; i < thread_num; i++)
	{
		pthread_t tid;
		threads.push_back(tid);
		inputs.push_back(new input_struct);
		inputs[i]->threadID = i;
		inputs[i]->query = query;
		cout << "calling thread " << char(i+65) << endl; 
		pthread_create(&threads[i],NULL,runner,inputs[i]);
	}
	// int counter = 0;
	// for (int i = 0; i < thread_num; i++)
	// // vector of abstracts
	// {
	// 	// if( i >= thread_num){/* all threads have jobs*/
	// 	// 	//wait for the next thread then create a new job for it
	// 	// 	pthread_join(threads[i%thread_num], NULL);//!!!!!!!!!!!!!!!!!!!!!! names are wrong
	// 	// 	//continue;
	// 	// }
	// 	//struct input_struct args;
	// 	logs.push_back(new input_struct);
	// 	//logs[i]->input_file_name = file_names[i];

	// 	logs[i]->file_names = &file_names;

	// 	logs[i]->output_file_name = outfile_name;
	// 	logs[i]->threadID = i;
	// 	logs[i]->query = query;
	// 	cout << "calling thread " << logs[i]->threadID << endl; 
	// 	pthread_create(&threads[i%thread_num],NULL,runner,logs[i]);
	// }
	for (int i = 0; i < thread_num; i++)
		pthread_join(threads[i], NULL);
	
	// for (int i = 0; i < num_abstr_to_scan; i++)
	// {
	// 	cout << logs[i]->input_file_name <<"---thread " << char(logs[i]->threadID+logs[i]->threadName) << endl;
	// }
	sort(logs.begin(), logs.end(), compareByLength);

	for (int i = 0; i < num_abstr_to_return; i++)
	{
		outfile << "###" << endl;
		outfile << "Result "<< i+1 << ":" << endl;
		outfile << "File: " << logs[i]->abstract_name << endl;
		//fprintf(outfile,"File: %.4f",logs[i]->similarity);
		outfile << setprecision(4) << fixed << "Score: " << logs[i]->similarity << endl;
		outfile << "Summary: " << logs[i]->summary << endl;	
	}

   	printf("Main thread exiting\n");
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

/* The threads will execute in this function */
void *runner(void *arguments) {
	// while calculating jaccard similarity save the best sentence to return
	//string file_name = *((string*) outfile_name);
	struct input_struct *args = (struct input_struct *)arguments;
	do
	{
	cout << "thread " << char(65 + args->threadID) << " working************" << endl;
		/* */
		// if(file_name_index == logs.size()){
		// 	cout << "thread exiting "  << char(65 + args->threadID) << endl;
		// 	//return output;
		// 	pthread_exit(NULL);
		// }
		/*	request file list access, get a file to process*/
		pthread_mutex_lock(&file_index_mutex);// file acquired
		cout << "================================" << file_name_index << endl;
		if(file_name_index==logs.size()){
			args->work = false;
			waiting_processes--;
			cout << "thread exiting "  << char(65 + args->threadID) <<  " Lastttttttttttttt"<<  endl;
			pthread_mutex_unlock(&file_index_mutex);
    		pthread_exit(NULL);		
		}
		else{
			args->input_file_name = logs[file_name_index]->abstract_name;
			args->current_file_index = file_name_index;
			//cout << "thread " << char(65 + args->threadID) << " acquired " << args->input_file_name << " --------------"<< endl;
			file_name_index++;
			ofstream outfile;
			outfile.open(outfile_name, std::ios::app);
			outfile << "Thread " << char(65 + args->threadID) << " is calculating " << args->input_file_name << endl;
			outfile.close();

			//pthread_mutex_unlock(&wait_mutex);
			waiting_processes--;
			//pthread_mutex_unlock(&wait_mutex);
		}
			pthread_mutex_unlock(&file_index_mutex);// file acquired

		/*Starting calculation*/
		jaccard(args);

		/*request file list access to log results*/
		pthread_mutex_lock(&file_list_mutex);
		logs[args->current_file_index]->similarity = args->jaccard;
		logs[args->current_file_index]->summary = args->summary;
		pthread_mutex_unlock(&file_list_mutex);

		// while (waiting_processes>0)
		// {
		// 	/* code */
		// }
		while(waiting_processes > 0){
			//cout << waiting_processes << endl;
		}

		pthread_mutex_lock(&file_index_mutex);
		if(file_name_index == logs.size())	
			args->work = false;
		else
			waiting_processes++;
		pthread_mutex_unlock(&file_index_mutex);
		
	} while(args->work);
		
	cout << "thread exiting "  << char(65 + args->threadID) << endl;
	//return output;
    pthread_exit(NULL);
}

void jaccard(input_struct *inputArgs)
{
	ifstream infile;
	string file_path = "../abstracts/" + inputArgs->input_file_name;
	//cout << file_path  << "---"<< endl;
	infile.open(file_path);
	string abstract;
	string line;
	while(getline(infile, line)){
		abstract += line;
	}
	string query = inputArgs->query;
	//cout << inputArgs->input_file_name << "abstract" << abstract << endl;
	vector<string> abstract_vector;
	vector <string> query_vector;
	vector<pair< vector<string>,int> > sentences;
	//get query set
	tokenize(query,&query_vector,' ');
	set<string> query_set(query_vector.begin(), query_vector.end());
	//get sentences
	tokenize(abstract,&abstract_vector,'.');

	set<string> intersection_set ;
	int best_match = 0;
	string best_sentence="";
	for (int i = 0; i < abstract_vector.size(); ++i)
	{
		vector<string> temp;
		tokenize(abstract_vector[i],&temp,' ');
		//cout << abstract_vector[i] << endl;
		set<string> temp_set(temp.begin(), temp.end());
		int matching_words = 0;
		//*****matching_words = similarity(query_set,temp_set);//number of intersecting words
		matching_words = similarity(query_set,temp_set,&intersection_set);//number of intersecting words
		//cout <<"--------" <<matching_words << "********" << endl;
		if(matching_words > best_match){//finding the best sentence
			best_match = matching_words;
			//best_sentence = abstract_vector[i];
			inputArgs->summary = abstract_vector[i];
		}
		sentences.push_back(make_pair(temp,matching_words));
	}
	//put all sentences in a set
	set<string> union_set = query_set;
	int total = 0;
	for (int i = 0; i < sentences.size(); ++i){
		for(auto w : sentences[i].first){
			union_set.insert(w);
		}
		//cout << sentences[i].second<< endl;
		total += sentences[i].second;// !!!!!!!!!!!!!!!!!!!!! same words in different sentences
	}
	//union_set.insert(query_set.begin(), query_set.end());
	union_set.insert(".");
	//cout << inputArgs->input_file_name << "---total-----"<< total << "----------------" << endl;
	//cout << union_set.size() << "union--------" << endl;
	double jaccard = double(total) / union_set.size();
	//cout <<  inputArgs->input_file_name <<"jaccard: " << jaccard << endl;
	inputArgs->jaccard = jaccard;

}

void tokenize(string inputStr, vector<string>* tokens, char tokenSeperator)
{
	// Vector of string to save tokens
    //vector <string> sentences;
     
    // stringstream class check1
    stringstream check1(inputStr);
     
    string token;
    //cout <<	inputStr << endl;
    // Tokenizing w.r.t. space ' '
    while(getline(check1, token, tokenSeperator))
    {
    	string sstemp = token;	
		stringstream ss(sstemp);
    	string word1;
   		ss >> word1;
   		//cout << word1 << endl;
   		if(!word1.empty()){
    	//cout << token <<  endl;
        tokens->push_back(token);
   		}
    }
    //cout << inputStr << endl;
    // for(int i = 0; i < tokens.size(); i++)
    //     cout << tokens[i] << '\n';
}

int similarity(set<string> query_set, set<string> abstract_set, set<string> *intersection_set){

	int intersectionOld = intersection_set->size();
	//auto word;
	for (auto word : query_set)
	{
		if(abstract_set.count(word)){
			//intersection++;
			intersection_set->insert(word);
		}
	}
	//cout << "intersection " << intersection << endl;
	//set<string> union_set = abstract_set;
	//union_set.insert(query_set.begin(), query_set.end());
	//cout << "union" << union_set.size() << endl;
	return intersection_set->size()-intersectionOld;
}
// int similarity(set<string> query_set, set<string> abstract_set){

// 	int intersection = 0;
// 	//auto word;
// 	for (auto word : query_set)
// 	{
// 		if(abstract_set.count(word)){
// 			intersection++;
			
// 		}
// 	}
// 	//cout << "intersection " << intersection << endl;
// 	set<string> union_set = abstract_set;
// 	union_set.insert(query_set.begin(), query_set.end());
// 	//cout << "union" << union_set.size() << endl;
// 	return intersection;
// }