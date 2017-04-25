#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>
#include <string.h>

using namespace std;
//capacity could be 70% favorable outcome
//capacity is the value that all threads commit to
#define num_threads 16
#define m_size 128

// g++ -lpthread -o masterslave masterSlaveGA.cpp
static bool check_fav(string fav, string a);
static void* count_fav( void* threadarg );
static void mutate(int i, int j, int index);
static void swap(struct elem a, struct elem b, int start);

vector< vector<string> > m_vspace;

struct elem{
    int x;
    int y;
    string dna;
};

struct thread_data {
    int thread_id;
    int local_count;
    string fav;
};

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
    struct thread_data thread_data_array[num_threads];
    pthread_t threads[num_threads];
    int rc;
    pthread_attr_t attr;
        
    //initialize attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
    printf("made it to after all init \n");
    while(!fav_capacity) 
    {
		cout << "beginning of while not capacity loop" << endl;
        vector<struct elem> matingPool;

        //copy over m_vspace into matingPool vector;
        for(int i=0; i<m_size; i++) {
            for(int j=0; j<m_size;  j++){
                struct elem e;
                e.x = i;
                e.y = j;
                e.dna = m_vspace[i][j];
                matingPool.push_back(e);
            }
        }
        printf("after pushback \n");
        while(matingPool.size() > 0) {
            //cout << "In mating while, Mating Pool size " << matingPool.size() << "\n";
            //crossover - for all in mating pool, pick a random partner, switch one random block
            int firstIndex = rand()%(matingPool.size());
            int secondIndex = rand()%(matingPool.size());
            while(firstIndex==secondIndex) {
                secondIndex = rand()%(matingPool.size());
            }
            //printf("first index = %d , second index = %d \n", firstIndex, secondIndex);
                 
            //Recombination - save the new values determined by crossover
            int blockID = rand()%numBlocks;
            swap(matingPool[firstIndex], matingPool[secondIndex], (blockID*chunk_size));
            //printf("outside of swap \n");
            
            //cout << "Mating Pool size before erases " << matingPool.size() << "\n";
            if(firstIndex > secondIndex) {
                matingPool.erase(matingPool.begin() + firstIndex);
                matingPool.erase(matingPool.begin() + secondIndex);
            }
            else {
                matingPool.erase(matingPool.begin() + secondIndex);
                matingPool.erase(matingPool.begin() + firstIndex);
            }
            //cout << "Mating Pool size after erases " << matingPool.size() << "\n";
        }//end of mating while loop - all paired up
        
		cout << "before mutate" << endl;
        //Mutation - 10% chance to flip one random bit
        for (int i=0; i < m_size; i++) {
            for(int j=0; j< m_size; j++){  
                int chance = rand()%100;
                if(chance < 10) {
                    int bit = rand()%n;
					//cout << "calling mutate on m_vspace i = " << i << " j = " << j <<  "index = " << bit << endl; 
					//cout << "m_vspace at that index = " << m_vspace[i][j] << endl;
                    mutate(i, j, bit);
					//cout << "new m_vspace at that index = " << m_vspace[i][j] << endl;
                }
            }
        }
        cout << "done with mutations!" << endl;
        
        //update capacity value by using slaves
        //split the m_size*m_size matrix into num_threads amount of threads
        for(int j = 0; j<num_threads; j++) {
            thread_data_array[j].thread_id = j;
            thread_data_array[j].local_count = 0;
            thread_data_array[j].fav = fav;
            rc = pthread_create(&threads[j], NULL, count_fav, (void *) &thread_data_array[j]);
            if(rc) {
                cout << "ERROR creating thread" << endl;
            }
        }
                                        
        //thread join
        for(int k = 0; k<num_threads; k++) {
            rc = pthread_join(threads[k], NULL);
			if(rc) {
                cout << "ERROR joining thread" << endl;
            }
        }
		cout << "after thread creates & joins" << endl;									
        //after thread join, decide population number
        double total_fav_pop = 0;
        for(int i =0; i<num_threads; i++) {
            total_fav_pop += thread_data_array[i].local_count;
        }
        //total_fav_pop now has all of favorable people
        //decide capacity
        //70% favorable
        double population = m_size * m_size;
        double cap = total_fav_pop / population;
        if(cap >= .7) {
            fav_capacity = true;
        }
		cout << "END OF THIS GENERATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cout << "capacity = " << cap << endl << endl;
    } //end of capacity while - all done with generations
	
    //print results
    for(int i = 0; i<m_size; i++){
        for(int j=0; j<m_size; j++){
            printf("%s", m_vspace[i][j].c_str());
        }
        cout<<endl;
    }
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
    
void mutate(int i, int j, int index)
{
    string localString = m_vspace[i][j];
	if(!check_fav(fav, localString)) {
		if (localString.at(index) == '1') {
			localString.at(index) = '0';
		}
		else {
			localString.at(index) = '1';
		}
		m_vspace[i][j] = localString;
	}
}
         
//THREAD FUNCTION
void* count_fav( void* threadarg )
{
    struct thread_data * mydata;
    mydata = (struct thread_data*) threadarg;
    int thread_index = mydata->thread_id; // i hope this is correct and works the same as before
    int start = thread_index *(m_size) / num_threads;
    int end = start + (m_size  / num_threads) ; // should be correct
    int local_count = 0;

    for(int i = start; i<end; i++) {
        for(int j=0; j<m_size; j++) {
            //check favorability of m_vspace[i];
            string favor = mydata->fav; // is this also going to work?  
            if(check_fav(favor, m_vspace[i][j])) {
                local_count++;
            }
        }
    }
    mydata->local_count = local_count; // assign the local count value to the thread?
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
