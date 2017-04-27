#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>
#include <string.h>

//FINISH THIS BY TONIGHT

using namespace std;
//capacity could be 70% favorable outcome
//capacity is the value that all threads commit to
#define num_threads 16
#define m_size 128

struct elem{
    int x;
    int y;
    string dna;
};

struct thread_data {
    int thread_id;
    vector<struct elem>  thread_matingPool;
    double fav_count;
    string fav;
};


// g++ -pthread -o masterslave masterSlaveGA.cpp
static bool check_fav(string fav, string a);
void *thread_calc( void* threadarg );
static void mutate(int i, int j, int index);
static void swap(struct elem a, struct elem b, int start);

vector< vector<string> > m_vspace;
struct thread_data thread_data_array[num_threads];
pthread_t threads[num_threads];


int chunk_size; //size of chunk
int n; // size of string length
string fav; ///give this to the threads
int numBlocks; //number of blocks of dna
bool fav_capacity = false;

// main function
int main(int argc, char** argv) 
{
	if(argc != 4) 
    {
        cout <<"usage: %s n chunck_size fav\n" <<  argv[0] << endl;
        return 0;
    }
    n = strtol(argv[1], NULL, 10);
    chunk_size = strtol(argv[2], NULL, 10);
    string arg3 (argv[3]);
    fav = arg3;
	numBlocks = n/chunk_size;
	cout << "n " << n << ", chunk_size " << chunk_size << ", fav = " << fav << ", numBlocks " << numBlocks << "\n" ;
    
    //Initial state of the matrix: even rows are bits of size n all 0’s, size of n bits all 1’s.
	//TODO: make the strings we push back relate to the input of n!!!!!
    for (int i =0; i < m_size; i++) {
        vector<string> myvector;
        for (int j = 0; j < m_size; j++) {
            if(i%2 == 0){
				myvector.push_back("11111111");
            }
            else {
				myvector.push_back("00000000");
            }
        }
        m_vspace.push_back(myvector);
    }
    	
    cout << "Done initializing m_vspace" << endl;
	
    //pthreads
    int rc;
    pthread_attr_t attr;
        
    //initialize attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
    printf("made it to after all init \n");
    while(!fav_capacity) 
    {
	    //do mating using the slaves
        //split the m_size*m_size matrix into num_threads amount of threads
        for(int q = 0; q<num_threads; q++) {
			//copy over m_vspace into m_vspace_copies
			for(int i=0; i< (q*m_size)/num_threads; i++) {
				for(int j=0; j<m_size;  j++){
					struct elem e;
					e.x = i;
					e.y = j;
					e.dna = m_vspace[i][j];
					thread_data_array[q].thread_matingPool.push_back(e);
                    thread_data_array[q].thread_id = q;
                    thread_data_array[q].fav = fav;
				}
			}
			rc = pthread_create(&threads[q], NULL, thread_calc, (void *) &thread_data_array[q]);
            if(rc) {
                cout << "ERROR creating thread" << endl;
            }
        }
        cout << "done with pthread creates" << endl;                                 
        //thread join
        for(int k = 0; k<num_threads; k++) {
            rc = pthread_join(threads[k], NULL);
			if(rc) {
                cout << "ERROR joining thread" << endl;
            }
        }
		cout << "after joins" << endl;

        double total_fav_pop = 0;
		for(int i = 0; i<num_threads; i++) {
            
            total_fav_pop += thread_data_array[i].fav_count;
            cout << "  thread_data_array[i] " <<  thread_data_array[i].fav_count << endl;
		}
        
        cout << "total_fav_pop = " << total_fav_pop << endl;
        
        //total_fav_pop now has all of favorable people
        //decide capacity  = 70% favorable
        double population = m_size * m_size;
        double cap = total_fav_pop / population;
        cout << "END OF THIS GENERATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cout << "capacity = " << cap << endl << endl;                   
        if(cap >= .7) {
            fav_capacity = true;
        }
    }
        //outside capacity while inside main
    return 0; 
}
                                                                  
// functions/////////
//recombination
void swap(struct elem a, struct elem b, int start)
{
    string temp = a.dna;
    for(int i = start; i < start+chunk_size;i++) {
        a.dna[i] = b.dna[i];
        b.dna[i] = temp[i];
    }
    //copys elem dna into the m_vspace[x][y]
	m_vspace[a.x][a.y] = a.dna;
	m_vspace[b.x][b.y] = b.dna;
}

void mutate(int thread_index, int i, int j, int index)
{
    string localString = thread_data_array[thread_index].thread_matingPool[i*m_size+j].dna;
	if(!check_fav(fav, localString)) {
		if (localString.at(index) == '1') {
			localString.at(index) = '0';
		}
		else {
			localString.at(index) = '1';
		}
		thread_data_array[thread_index].thread_matingPool[i*m_size+j].dna = localString;
	}
}

//THREAD FUNCTION
void *thread_calc( void* threadarg ) 
{
    
    int fav_count = 0;
    struct thread_data * mydata;
    mydata = (struct thread_data*) threadarg;
    string fav = mydata->fav;
    
    int thread_index = mydata->thread_id;
    vector<struct elem>  thread_matingPool = mydata->thread_matingPool;

	while(thread_matingPool.size() > 0) {
		//crossover - for all in mating pool, pick a random partner, switch one random block
        int firstIndex = rand()%(thread_matingPool.size());
		int secondIndex = rand()%(thread_matingPool.size());
		while(firstIndex==secondIndex) {
			secondIndex = rand()%(thread_matingPool.size());
		}
		//printf("first index = %d , second index = %d \n", firstIndex, secondIndex);
			 
		//Recombination - save the new values determined by crossover
		int blockID = rand()%numBlocks;
		swap(thread_matingPool[firstIndex], thread_matingPool[secondIndex], (blockID*chunk_size));
		//printf("outside of swap \n");
		
        
        
        /*Mutate First Index*/
        int chance1= rand()%100;
        if(chance1 < 10) {
            int bit = rand()%n;
            mutate(thread_index, thread_matingPool[firstIndex].x, thread_matingPool[firstIndex].y, bit);
        }
        
        /*Mutate Second Index*/
        int chance2= rand()%100;
        if(chance2 < 10) {
            int bit = rand()%n;
            mutate(thread_index, thread_matingPool[secondIndex].x, thread_matingPool[secondIndex].y, bit);
        }
        
        
        
        
        
        /*Copy First Index back into m_vspace */
        int i = thread_matingPool[firstIndex].x;
        int j = thread_matingPool[firstIndex].y;
        string dna1 =thread_matingPool[firstIndex].dna;
        //cout << "m_vspace[" << i << "][" << j << "] was before : " << m_vspace[i][j] << endl;
        m_vspace[i][j] = dna1;
      //  cout << "m_vspace[" << i << "][" << j << "] is now : " << m_vspace[i][j] << endl;
        
        
        /*Copy secondIndex back into m_vspace */
        int x = thread_matingPool[secondIndex].x;
        int y = thread_matingPool[secondIndex].y;
        string dna2 =thread_matingPool[secondIndex].dna;
       // cout << "m_vspace[" << x << "][" << y << "] was before : " << m_vspace[x][y] << endl;

        m_vspace[x][y] = dna2;
        //cout << "m_vspace[" << x << "][" << y << "] is now : " << m_vspace[x][y] << endl;

        
        /*Check favorabiltiy for first Index */
        if(check_fav(fav, dna1)){
            fav_count++;
        }
        /*Check favorabiltity for second Index*/
        if(check_fav(fav,dna2)){
            fav_count++;
        }
        
        
		//cout << "Thread Mating Pool size before erases " << thread_matingPool.size() << "\n";
		if(firstIndex > secondIndex) {
			thread_matingPool.erase(thread_matingPool.begin() + firstIndex);
			thread_matingPool.erase(thread_matingPool.begin() + secondIndex);
		}
		else {
			thread_matingPool.erase(thread_matingPool.begin() + secondIndex);
			thread_matingPool.erase(thread_matingPool.begin() + firstIndex);
		}
	
	}//end of mating while loop - all paired up                                  
        
    //we need to check favorability now
    
	// recopy the updated array to the thread data array
	mydata->thread_matingPool = thread_matingPool;
    mydata->fav_count = fav_count;
    
    
    cout << "made it to exit in thread: " << thread_index << " fav_count = " << fav_count <<  endl;

    pthread_exit(NULL);
    
}

//finds favorable trait inside the bit string of length n
bool check_fav(string fav, string a )
{
    for(int j=0; j<numBlocks; j++) {
        int sameBits = 0;
        for(int k=0; k<chunk_size; k++) {
            int index = j * chunk_size + k;
            if(a[index] == fav[k]) {
                sameBits++;
            }
        }
        if(sameBits == chunk_size) {
            return true;
        }
    }
    return false;
}
