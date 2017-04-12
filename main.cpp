

//capacity could be 70% favorable outcome
//capacity is the value that all threads commit to 
#define num_threads 16; 


char**  [m_space*m_space] m_space;
int chunk_size; //size of chunk 
int n; // size of string length 
char[chunk_size] fav; ///give this to the threads 

int numBlocks = n/chunk_size; 

bool fav_capacity = false;

void main() {
	//Initial state of the matrix: even rows are bits of size n all 0’s, size of n bits all 1’s. 
	for(int i =0; i<m_size; i++){
		for(int j = 0; j<m_size; j++){
			if(i%2 == 0) { //row is even 
				Bits of size All 0’s; 
			} else { 
				Bits of size All 1’s 
			} 
		}
	}

	while(!fav_capacity) {
		//crossover - for all in mating pool, pick a random partner, switch one random block
		vector<char**> matingPool [m_size*m_size];
		for(ink =0; k<m_size/2 ; k++){
			
			//crossover 
			int firstIndex = rand % m_size^2;
			int secondIndex = rand % m_size^2;
			char* m1 = matingPool.at(firstIndex);
			char* m2 = matingPool.at(secondIndex); 

			//Recombination - save the new values determined by crossover
			int blockID = rand%numBlocks;
			swap(m1, m2, block_swap*chunk_size);

			matingPool.remove[firstIndex ];
			matingPool.remove[secondIndex  ];
		}

		//Mutation - rare chance to flip one bit
		//10 % of people get mutated every iteration
		for (int i = 0; i < m_size^2; i++) {
			int chance = rand%100;
			if(chance < 10) {
				int bit = rand%n;
				mutate(m_size[i], bit);
			}
		}
		//update capacity value by using slaves
		//split the m_size*m_size matrix into num_threads amount of threads
		for(int j = 0; j<num_threads; j++){
			Thread[j] gets (thread_index); 
		}

		//thread join 

		//after thread join, decide population number 
		double total_fav_pop = 0;
		for(int i =0; i<num_threads; i++){
			Total_fav_pop += threads[i].local_count; 
		}

		//total_fav_pop now has all of favorable people 

		//decide capacity 
		//70% favorable 
		Double population = m_size * m_size; 
		Double cap = total_fav_pop / population; 

		if(cap >= .7) {
			fav_capacity = true; 
		}
	}
}
void swap(char** a, char** b, int start){
	char* temp = a;
	for(i = start; i < start+chunk_size;i++) {
		a[i] = b[i]; 
		b[i] = temp[i];  
}


void mutate(char** a, index) {
	a[index] = ~a[index];
}


//THREAD FUNCTION 
void count_fav( int thread_index ) {
	int start = thread_index *(m_size * m_size ) / num_threads;
	int end = start + (m_size * m_size ) / num_threads; 
	int local_count = 0; 

	for(int i = start; i<end; i++){
		//check favorability of m_space[i]; 
		char*[chunk_size] my_string = m_space[i]; 

		if(check_fav(my_string)) {
			local_count++;
		}
	}
}

bool check_fav(char* fav, char* a ){
	for(int j=0; j<n/chunk_size; j++){
		int sameBits = 0;
		for(k=0; k<chunk_size; k++){
			int index = j * chunk_size + k; 
			if(a[index] == fav[k]){
				sameBits++;
			}
		}
		if(sameBits == chunk_size){
			return true;	    
		}
	}
	return false;
}

