#include <stdio.h>
#include <bits/stdc++.h>
#include <vector>
#include <iostream>     // std::cout
#include <algorithm>    // std::shuffle
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <limits>

#define SWAP 0
#define INSERTION 1
#define CHANGE_MACHINE 2
#define CHANGE_ROBOT 3

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
    void swap(Instance * inst, int a, int b);
    void insertion(Instance * inst, int a, int b);
    void next_machine(Instance * inst, int id);
    void next_robot(Instance * inst, int id);
    Solution copy();
};

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

    //Correction required when the operations have different numbers of alternative machines
    this->machines[i] %= inst->num_alt_machines[current_job][next_operation];

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

// Greedy heuristic algorithm for resource assignment (Homayouni & Fontes, 2020)
Solution generate_initial_solution(Instance * inst)
{

  Solution solution;

  for (int i = 0; i < inst->num_jobs; i++) {
    for (int j = 0; j < inst->num_operations[i]; j++) {
      solution.operations.push_back(i);
    }
  }
  solution.makespan = 0;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  shuffle (solution.operations.begin(), solution.operations.end(), std::default_random_engine(seed));

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
  int current_operation;

  int selected_robot;
  int time_available_robot;
  int waiting_time;
  int aux_time;
  int aux_waiting;
  int departure_time;

  int id_selected_machine;
  int selected_machine;
  int time_available_machine;
  int aux_machine;

  for (int i = 0; i < inst->num_total_operations; i++)
  {
    current_job = solution.operations[i];

    //Robot assignment => VF1 - Table 1 and V3 - Table 2
    selected_robot = 0;
    pos_current_job = position_jobs[current_job];
    time_available_robot = time_robots[0] + inst->layout[position_robots[0]][pos_current_job];
    waiting_time = abs( time_available_robot - time_jobs[current_job] );

    for (int r = 1; r < inst->num_robots; r++)
    {
      aux_time = time_robots[r] + inst->layout[position_robots[r]][pos_current_job];
      aux_waiting = abs( aux_time - time_jobs[current_job] );
      if(aux_waiting < waiting_time)
      {
        selected_robot = r;
        time_available_robot = aux_time;
        waiting_time = aux_waiting;
      }
    }

    departure_time = max(time_available_robot, time_jobs[current_job]);

    //Machine assignment => MF1 - Table 1 and M4 - Table 2
    id_selected_machine = 0;
    current_operation = count_jobs[current_job];
    selected_machine = inst->jobs[current_job][current_operation][0][0];
    time_available_machine = max(time_machines[selected_machine], departure_time + inst->layout[pos_current_job][selected_machine]) + inst->jobs[current_job][current_operation][1][0];

    for (int m = 1; m < inst->num_alt_machines[current_job][current_operation]; m++)
    {
      aux_machine = inst->jobs[current_job][current_operation][0][m];
      aux_time = max(time_machines[aux_machine], departure_time + inst->layout[pos_current_job][aux_machine]) + inst->jobs[current_job][current_operation][1][m];
      if(aux_time < time_available_machine)
      {
        id_selected_machine = m;
        selected_machine = aux_machine;
        time_available_machine = aux_time;
      }
    }

    if( selected_machine != pos_current_job )
    {
      time_robots[selected_robot] = departure_time + inst->layout[pos_current_job][selected_machine];
      position_robots[selected_robot] = selected_machine;
      time_machines[selected_machine] = time_available_machine;
    } else {
      time_machines[selected_machine] = time_machines[selected_machine] + inst->jobs[current_job][current_operation][1][id_selected_machine];
    }

    solution.machines.push_back(id_selected_machine);
    solution.robots.push_back(selected_robot);

    position_jobs[current_job] = selected_machine;
    time_jobs[current_job] = time_machines[selected_machine];
    count_jobs[current_job]++;
    solution.makespan = max(solution.makespan, time_machines[selected_machine]);

  }

  return solution;

}

Solution find_best_improvement(Instance * inst, Solution best_solution, int neighborhood)
{

  Solution neighbor_solution = best_solution.copy();
  int previous_makespan = neighbor_solution.makespan;

  if(neighborhood == SWAP)
  {
    int aux;
    for (int a = 0; a < inst->num_total_operations; a++) {
      for (int b = a+1; b < inst->num_total_operations; b++) {

        if(neighbor_solution.operations[a] == neighbor_solution.operations[b])
          continue;

        aux = neighbor_solution.operations[a];
        neighbor_solution.operations[a] = neighbor_solution.operations[b];
        neighbor_solution.operations[b] = aux;

        neighbor_solution.update_makespan(inst);

        if ( neighbor_solution.makespan < best_solution.makespan ) {
          best_solution = neighbor_solution.copy();
        }

        neighbor_solution.operations[b] = neighbor_solution.operations[a];
        neighbor_solution.operations[a] = aux;
        neighbor_solution.makespan = previous_makespan;

      }
    }
  }

  //This can be improved with iterating consecutive swaps
  else if(neighborhood == INSERTION)
  {
    int aux;
    std::vector<int>::iterator begin = neighbor_solution.operations.begin();

    for (int a = 0; a < inst->num_total_operations; a++) {
      for (int b = a+2; b < inst->num_total_operations; b++) {

        aux = neighbor_solution.operations[a];
        neighbor_solution.operations.erase(begin + a);
        neighbor_solution.operations.insert(begin + b, aux);

        neighbor_solution.update_makespan(inst);

        if ( neighbor_solution.makespan < best_solution.makespan ) {
          best_solution = neighbor_solution.copy();
        }

        neighbor_solution.operations.erase(begin + b);
        neighbor_solution.operations.insert(begin + a, aux);
        neighbor_solution.makespan = previous_makespan;

      }
    }
  }

  else if(neighborhood == CHANGE_MACHINE)
  {

    int aux;
    int count_jobs[inst->num_jobs] = {0};
    int current_job;
    int current_operation;

    for (int i = 0; i < inst->num_total_operations; i++) {

      current_job = neighbor_solution.operations[i];
      current_operation = count_jobs[current_job];
      aux = neighbor_solution.machines[i];

      for (int k = 0; k < inst->num_alt_machines[current_job][current_operation] - 1; k++) {

        neighbor_solution.machines[i]++;
        //neighbor_solution.machines[i] %= inst->num_alt_machines[current_job][current_operation];
        neighbor_solution.update_makespan(inst);

        if ( neighbor_solution.makespan < best_solution.makespan ) {
          best_solution = neighbor_solution.copy();
        }
      }

      neighbor_solution.machines[i] = aux;
      neighbor_solution.makespan = previous_makespan;
      count_jobs[current_job]++;
    }
  }

  else if(neighborhood == CHANGE_ROBOT)
  {
    int aux;
    for (int i = 0; i < inst->num_total_operations; i++) {

      aux = neighbor_solution.robots[i];
      for (int k = 0; k < inst->num_robots - 1; k++) {

        neighbor_solution.robots[i]++;
        neighbor_solution.robots[i] %= inst->num_robots;
        neighbor_solution.update_makespan(inst);

        if ( neighbor_solution.makespan < best_solution.makespan ) {
          best_solution = neighbor_solution.copy();
        }
      }
      neighbor_solution.robots[i] = aux;
      neighbor_solution.makespan = previous_makespan;
    }
  }

  return best_solution;

}

// RVND-based local search
Solution local_search(Instance * inst, Solution solution)
{
  vector<int> neighborhoods = {SWAP, INSERTION, CHANGE_MACHINE, CHANGE_ROBOT};
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  shuffle (neighborhoods.begin(), neighborhoods.end(), std::default_random_engine(seed));

  int previous_makespan = solution.makespan;
  int k = 0;

  while (k < 4) {

    solution = find_best_improvement(inst, solution, neighborhoods[k]);
    if(solution.makespan < previous_makespan)
    {
      seed = std::chrono::system_clock::now().time_since_epoch().count();
      shuffle (neighborhoods.begin(), neighborhoods.end(), std::default_random_engine(seed));
      k = 0;
      previous_makespan = solution.makespan;

    } else {
      k++;
    }
  }
  return solution;
}

Solution perturbation(Instance * inst, Solution solution, int num_moves)
{
  int perturb_neighhoods[3] = {SWAP, CHANGE_MACHINE, CHANGE_ROBOT};
  srand (std::chrono::system_clock::now().time_since_epoch().count());
  int neighborhood = perturb_neighhoods[rand() % 3];

  int id_1;
  int id_2;
  int aux;
  if(neighborhood == SWAP)
  {
    int i = 0;
    while (i < num_moves) {

      srand (std::chrono::system_clock::now().time_since_epoch().count());
      id_1 = rand() % inst->num_total_operations;
      srand (std::chrono::system_clock::now().time_since_epoch().count());
      id_2 = rand() % inst->num_total_operations;

      if(solution.operations[id_1] != solution.operations[id_2])
      {
        aux = solution.operations[id_1];
        solution.operations[id_1] = solution.operations[id_2];
        solution.operations[id_2] = aux;
        i++;
      }
    }
  }
  else if(neighborhood == CHANGE_MACHINE)
  {
    for (int i = 0; i < num_moves; i++) {
      srand (std::chrono::system_clock::now().time_since_epoch().count());
      id_1 = rand() % inst->num_total_operations;
      solution.machines[id_1]++;
    }
  }
  else
  {
    for (int i = 0; i < num_moves; i++) {
      srand (std::chrono::system_clock::now().time_since_epoch().count());
      id_1 = rand() % inst->num_total_operations;
      solution.robots[id_1]++;
      solution.robots[id_1] %= inst->num_robots;
    }
  }
  solution.update_makespan(inst);

  return solution;
}

Solution multistart_iterated_local_search(Instance * inst, int num_restarts, int max_iter, int num_moves)
{
  Solution best_solution;
  best_solution.makespan = INT_MAX;

  Solution current_better_solution;
  Solution solution;
  int iter;

  for (int i = 0; i < num_restarts; i++) {

    solution = generate_initial_solution(inst);
    current_better_solution = solution.copy();
    iter = 0;

    while ( iter < max_iter )
    {
      solution = local_search(inst, solution);
      if(solution.makespan < current_better_solution.makespan)
      {
        current_better_solution = solution.copy();
        iter = 0;
      }
      solution = perturbation(inst, solution, num_moves);
      iter++;
    }

    if( current_better_solution.makespan < best_solution.makespan )
      best_solution = current_better_solution;
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

  int num_restarts = 10;
  int max_iter = 300;
  int num_moves = 3;

  Instance instance = read_instance();

  time_t start, end;
  int trials = 30;

  printf("makespan,elapsed_time\n");

  for (int i = 0; i < trials; i++) {

    time(&start);

    Solution solution = multistart_iterated_local_search(&instance, num_restarts, max_iter, num_moves);

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

  // printf("Makespan: %d\n", solution.makespan);
  //
  // // Calculating total time taken by the program.
  // double time_taken = double(end - start);
  // cout << "Time taken by program is : " << fixed
  //      << time_taken << setprecision(5);
  // cout << " sec " << endl;

}
