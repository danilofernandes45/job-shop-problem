#include <stdio.h>
#include <bits/stdc++.h>
#include <vector>
#include <iostream>     // std::cout
#include <algorithm>    // std::shuffle
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

using namespace std;

class Instance {
  public:
    int num_jobs;
    int **** jobs;
    int num_total_operations;
    int * num_operations;
    int num_machines;
    int ** num_alt_machines;
    int ** layout;
    int num_robots;

};

class Solution {
  public:
    vector<int> operations;
    vector<int> machines;
    vector<int> robots;
    int makespan;

    void update_makespan(Instance * inst);
    Solution copy();
};

//O(n), n -> num_total_operations
void Solution::update_makespan(Instance * inst)
{
  //Control variables
  int time_machines[inst->num_machines] = {0};
  int time_robots[inst->num_robots] = {0};
  int position_robots[inst->num_robots] = {0};
  int count_jobs[inst->num_jobs] = {0};
  int position_jobs[inst->num_jobs] = {0};
  int time_jobs[inst->num_jobs] = {0};

  //Tmp variables
  int current_job;
  int pos_current_job;
  int next_operation;

  int next_machine;
  int departure_time;
  int pos_robot;

  this->makespan = 0;

  for (int i = 0; i < inst->num_total_operations; i++) {

    current_job = this->operations[i];
    pos_current_job = position_jobs[current_job];
    next_operation = count_jobs[current_job];

    //Reparement on machines index, when it required
    if( this->machines[i] >= inst->num_alt_machines[current_job][next_operation] )
    {
      srand (std::chrono::system_clock::now().time_since_epoch().count());
      this->machines[i] = rand() % inst->num_alt_machines[current_job][next_operation];
    }

    next_machine = inst->jobs[current_job][next_operation][0][ this->machines[i] ];

    if(next_machine == pos_current_job)
    {
      time_machines[ next_machine ] += inst->jobs[current_job][next_operation][1][ this->machines[i] ];
    } else {
      pos_robot = position_robots[this->robots[i]];
      departure_time = max(time_jobs[current_job], time_robots[this->robots[i]] + inst->layout[pos_robot][pos_current_job]);
      time_robots[this->robots[i]] = departure_time + inst->layout[pos_current_job][next_machine];
      position_robots[this->robots[i]] = next_machine;

      time_machines[next_machine] = max(time_machines[next_machine], time_robots[this->robots[i]]);
      time_machines[next_machine] += inst->jobs[current_job][next_operation][1][ this->machines[i] ];
    }

    time_jobs[current_job] = time_machines[next_machine];
    position_jobs[current_job] = next_machine;
    count_jobs[current_job]++;

    this->makespan = max(this->makespan, time_machines[next_machine]);

  }

}

Solution Solution::copy()
{
  Solution copy;
  copy.operations = this->operations; //Attribution makes a copy of a vector
  copy.machines = this->machines;
  copy.robots = this->robots;
  copy.makespan = this->makespan;
  return copy;
}

// Generate a random feasible initial solution (Homayouni & Fontes, 2021)
Solution generate_initial_solution(Instance * inst)
{

  Solution solution;

  for (int i = 0; i < inst->num_jobs; i++) {
    for (int j = 0; j < inst->num_operations[i]; j++) {
      solution.operations.push_back(i);

      solution.machines.push_back( inst->num_machines ); // Generate a reparement in update_makespan

      srand (std::chrono::system_clock::now().time_since_epoch().count());
      solution.robots.push_back( rand() % inst->num_robots );
    }
  }

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  shuffle (solution.operations.begin(), solution.operations.end(), std::default_random_engine(seed));

  solution.update_makespan(inst);
  return solution;
}

Solution generate_neighbor(Instance * inst, Solution * solution)
{
  Solution neighbor = solution->copy();
  int i = 0;
  int aux, id_1, id_2;
  while ( i == 0 ) {

    srand (std::chrono::system_clock::now().time_since_epoch().count());
    id_1 = rand() % inst->num_total_operations;
    srand (std::chrono::system_clock::now().time_since_epoch().count());
    id_2 = rand() % inst->num_total_operations;

    if(neighbor.operations[id_1] != neighbor.operations[id_2])
    {
      aux = neighbor.operations[id_1];
      neighbor.operations[id_1] = neighbor.operations[id_2];
      neighbor.operations[id_2] = aux;
      i++;
    }
  }

  srand (std::chrono::system_clock::now().time_since_epoch().count());
  id_1 = rand() % inst->num_total_operations;
  neighbor.machines[id_1] = inst->num_machines; // Generate a reparement in update_makespan

  srand (std::chrono::system_clock::now().time_since_epoch().count());
  id_1 = rand() % inst->num_total_operations;
  srand (std::chrono::system_clock::now().time_since_epoch().count());
  neighbor.robots[id_1] = rand() % inst->num_robots;

  neighbor.update_makespan(inst);
  return neighbor;
}

Solution late_acceptance_hill_climbing(Instance * inst, int H, int max_iter)
{
  Solution solution = generate_initial_solution(inst);
  Solution best_solution = solution.copy();
  vector<int> history_list (H);
  fill (history_list.begin(), history_list.end(), solution.makespan);

  int i = 0;
  int i_idle = 0;
  Solution neighbor;

  while( i < max_iter || i_idle < 0.02*i )
  {
    neighbor = generate_neighbor(inst, &solution);

    if( neighbor.makespan >= solution.makespan )
    {
      i_idle++;
    } else {
      i_idle = 0;
    }

    if( neighbor.makespan < best_solution.makespan )
    {
      best_solution = neighbor.copy();
    }

    if( neighbor.makespan < history_list[ i % H ] || neighbor.makespan <= solution.makespan )
    {
      solution = neighbor;
    }
    if( neighbor.makespan < history_list[ i % H ] )
    {
      history_list[ i % H ] = neighbor.makespan;
    }

    i++;
  }

  return best_solution;
}

Instance read_instance()
{
  Instance instance;

  scanf("%d", &instance.num_jobs);

  instance.jobs = new int***[instance.num_jobs];

  instance.num_operations = new int[instance.num_jobs];

  instance.num_alt_machines = new int*[instance.num_jobs];

  instance.num_total_operations = 0;

  for (int i = 0; i < instance.num_jobs; i++) {
    scanf("%d", &instance.num_operations[i]);
    instance.num_total_operations = instance.num_total_operations + instance.num_operations[i];
    instance.num_alt_machines[i] = new int[instance.num_operations[i]];
    instance.jobs[i] = new int ** [instance.num_operations[i]];

    for (int j = 0; j < instance.num_operations[i]; j++) {
      scanf("%d", &instance.num_alt_machines[i][j]);
      instance.jobs[i][j] = new int*[2];
      instance.jobs[i][j][0] = new int[instance.num_alt_machines[i][j]];
      instance.jobs[i][j][1] = new int[instance.num_alt_machines[i][j]];

      for (int m = 0; m < instance.num_alt_machines[i][j]; m++) {
        scanf("%d %d", &instance.jobs[i][j][0][m], &instance.jobs[i][j][1][m]);
      }
    }
  }

  scanf("%d", &instance.num_machines);

  instance.layout = new int*[instance.num_machines];

  for (int i = 0; i < instance.num_machines; i++) {
    instance.layout[i] = new int[instance.num_machines];
    for (int j = 0; j < instance.num_machines; j++) {
      scanf("%d", &instance.layout[i][j]);
    }
  }

  scanf("%d", &instance.num_robots);

  return instance;
}

int main()
{

  int H = 1000;
  int max_iter = 100000;

  Instance instance = read_instance();

  time_t start, end;
  int trials = 30;

  printf("makespan,elapsed_time\n");

  for (int i = 0; i < trials; i++) {

    time(&start);

    Solution solution = late_acceptance_hill_climbing(&instance, H, max_iter);

    time(&end);

    printf("%d, %.2f\n", solution.makespan, double(end - start));

  }

  // for (int i = 0; i < instance.num_total_operations; i++) {
  //   printf("%d ", solution.operations[i]);
  // }
  // printf("\n");
  //
  // for (int i = 0; i < instance.num_total_operations; i++) {
  //   printf("%d ", solution.machines[i]);
  // }
  // printf("\n");
  //
  // for (int i = 0; i < instance.num_total_operations; i++) {
  //   printf("%d ", solution.robots[i]);
  // }
  // printf("\n");
  //
  // printf("Makespan: %d\n", solution.makespan);
  //
  // // Calculating total time taken by the program.
  // double time_taken = double(end - start);
  // cout << "Time taken by program is : " << fixed
  //      << time_taken << setprecision(5);
  // cout << " sec " << endl;

}
