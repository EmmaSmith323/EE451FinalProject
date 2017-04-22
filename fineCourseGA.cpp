#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>

using namespace std;
//capacity could be 70% favorable outcome
//capacity is the value that all threads commit to
#define num_threads 16
#define m_size 128


// g++ -lrt -lpthread -o finecourse fineCourseGA.cpp
static bool check_fav(char** fav, char* a);
static void* thread_calc( void* threadarg );
static void mutate(char* a,int index);
static void swap(char** a, char** b, int start);


struct thread_data {
    int thread_id;
	char ** m_space_copy;
	
};



char** m_space;
int chunk_size; //size of chunk
int n; // size of string length
char** fav; ///give this to the threads

int numBlocks;

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
    //TODO: is fav right?
    fav[0] = argv[3];
    numBlocks = n/chunk_size;
    
    //malloc m_space
    m_space = (char**) malloc(sizeof(char*)*m_size*m_size);
    
    //pthreads
    struct thread_data thread_data_array[num_threads];
    pthread_t threads[num_threads];
    int rc;
    pthread_attr_t attr;
    
    
    //initialize attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Initial state of the matrix: even rows are bits of size n all 0’s, size of n bits all 1’s.
    int i;
    for(int i =0; i<m_size*m_size; i++)
    {
        m_space[i] = (char*)malloc((n*sizeof(char)));

        if(i%2 == 0) 
        { //row is even
            for( int q = 0; q < n; q++) 
            {
                m_space[i][q] = '1';
            }
        } 
        else 
        {
            for(int q = 0; q < n; q++) 
            {
                m_space[i][q] = '0';
            }
        }
    }
                                                                    
    while(!fav_capacity) 
    {
        
        //DO we create and join every time in the loop, or just once at the beginning????????????????????????????
        //update capacity value by using slaves
        //split the m_size*m_size matrix into num_threads amount of threads
        for(int j = 0; j<num_threads; j++)
        {
			int start = j *(m_size * m_size ) / num_threads;
			int end = start + (m_size * m_size ) / num_threads -1;
			
			std::copy(m_space+start, m_space+end, thread_data_array[j].m_space_copy);
		
            rc = pthread_create(&threads[j], NULL, thread_calc, (void *) &thread_data_array[j]);
            if(rc) 
            {
                cout << "ERROR creating thread" << endl;
            }
        }
                                        
        //thread join
 
        for(int k = 0; k<num_threads; k++)
        {
            rc = pthread_join(threads[k], NULL);
           if(rc) 
            {
                cout << "ERROR joining thread" << endl;
            }
        }
		
		//Copying back each piece from the threads into the segment of m_space
		for(int k = 0; k < num_threads; k++) {
			int start = k * (m_size*m_size)/num_threads;
			int end = start + (m_size*m_size)/num_threads -1;
			for(int x = 0; x < (m_size*m_size)/num_threads; x++) {
				for(int q = start; q <= end;q++) {
					m_space[q] = thread_data_array[k].m_space_copy[x];
				}
			}
		}
		
             
        double total_fav_pop = 0;
		for(int i = 0; i<m_size*m_size; i++)
		{
			if(check_fav(fav, m_space[i])) 
			{
				total_fav_pop++;
			}
		}
   		 
                                    
        //total_fav_pop now has all of favorable people
                                    
        //decide capacity
        //70% favorable
        double population = m_size * m_size;
        double cap = total_fav_pop / population;
                                    
        if(cap >= .7) 
        {
            fav_capacity = true;
        }
        
            //inside while
        
    }
        //outside while inside main
    return 0; 
}
                                                                  
// functions/////////
//recombination
void swap(char** a, char** b, int start)
{
    char** temp = a;
    for(int i = start; i < start+chunk_size;i++) 
    {
        a[i] = b[i];
        b[i] = temp[i];
    }
}
                                                                           
void mutate(char* a, int index) 
{
    a[index] = ~a[index];
}
                                                                               
//THREAD FUNCTION
void *thread_calc( void* threadarg ) 
{
    struct thread_data * mydata;
    mydata = (struct thread_data*) threadarg;

    int thread_index = mydata->thread_id; // i hope this is correct and works the same as before
	// USe a char** or an array????????????????????????????????????????
	char** m_space_copy = mydata->m_space_copy;

	vector<char*> matingPool;
	vector<int>matingIndex;

	int temp_size = m_size*m_size/num_threads;
	
	//copy over m_space into matingPool vector;        
	for(int k=0; k<temp_size; k++)
	{
		//populate
		matingPool.push_back(m_space_copy[k]);
		matingIndex.push_back(k);
	}
	
	for(int k =0; k<temp_size/2 ; k++)
	{
										
		//crossover - for all in mating pool, pick a random partner, switch one random block
		int firstIndex = rand()%(temp_size);
		int secondIndex = rand()%(temp_size);
		while(firstIndex==secondIndex) 
		{
			secondIndex = rand()%(temp_size);
		}
		char*m1;
		char*m2;
		
		//Iterators
		vector<char*>::iterator it= matingPool.begin();
		vector<int>::iterator iter = matingIndex.begin();
		
		for( ; iter != matingIndex.end(); iter++)
		{
			
			if(*iter == firstIndex)
			{
				m1 = *it;
			}
			
			if(*iter == secondIndex)
			{
				m2 = *it;
			}
			
			it++;
		}
		//Recombination - save the new values determined by crossover
		int blockID = rand()%numBlocks;
		swap(&m1, &m2, (blockID*chunk_size));
	  
		vector<char*>::iterator ita = matingPool.begin();
		for( ; ita != matingPool.end(); ita++)
		{
			if(*ita == m1) // this should work okay im not sure yet
			{
				matingPool.erase(ita);
			}
			
			if((*ita) == m2)
			{
				matingPool.erase(ita);
			}
		}
	
		temp_size -= 2;
            
        }
                                        
        
        //Mutation - 10% chance to flip one random bit
        for (int i = 0; i < m_size*m_size/num_threads; i++) 
        {
            int chance = rand()%100;
            if(chance < 10) 
            {
                int bit = rand()%n;
                mutate(m_space_copy[i], bit);
            }
        }
        // recopy the updated array to the thread data array
        mydata->m_space_copy = m_space_copy;

    
}
        
//finds favorable trait inside the bit string of length n
bool check_fav(char** fav, char* a )
{
    for(int j=0; j<n/chunk_size; j++)
    {
        int sameBits = 0;

        for(int k=0; k<chunk_size; k++)
        {
            int index = j * chunk_size + k;
            if(a[index] == *fav[k])
            {
                sameBits++;
            }
        }
        if(sameBits == chunk_size)
        {
            return true;
        }
    }
    return false;
}