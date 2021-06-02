import pandas as pd
import numpy as np

milp = [134, 114, 120, 114, 94, 138, 110, 178, 144, 174]

ils_gap = []
ils_per_sd = []
ils_mean_time = []
lahc_gap = []
lahc_per_sd = []
lahc_mean_time = []

print("Instance  Multistart ILS              LAHC")
print("          C_max^best - C_max^avg - \\sigma - T - C_max^best - C_max^avg - \\sigma - T")

for i in range(2):
    ils_data = pd.read_csv("Experiments_Outputs/Homayouni_2021/ILS/out_mkt0"+str(i+1)+".csv")
    lahc_data = pd.read_csv("Experiments_Outputs/Homayouni_2021/LAHC/out_mkt0"+str(i+1)+".csv")

    ils_min = ils_data['makespan'].min()
    ils_mean = ils_data['makespan'].mean()
    #ils_gap.append( ( ils_min - milp[i] ) / milp[i] * 100 )
    ils_per_sd.append( np.sqrt( ils_data['makespan'].var() ) / ils_data['makespan'].mean() * 100 )
    ils_mean_time.append( ils_data['elapsed_time'].mean() )

    lahc_min = lahc_data['makespan'].min()
    lahc_mean = lahc_data['makespan'].mean()
    #lahc_gap.append( ( lahc_min - milp[i] ) / milp[i] * 100 )
    lahc_per_sd.append( np.sqrt( lahc_data['makespan'].var() ) / lahc_data['makespan'].mean() * 100 )
    lahc_mean_time.append( lahc_data['elapsed_time'].mean() )

    #print("fjsp"+str(i+1)+" & %d & %d & %.2f & %.2f & %.2f & %d & %.2f & %.2f & %.2f\\\\"%( milp[i], ils_min, ils_gap[i], ils_per_sd[i], ils_mean_time[i], lahc_min, lahc_gap[i], lahc_per_sd[i], lahc_mean_time[i] ) )
    print("MKT0"+str(i+1)+" & %d & %.2f & %.2f & %.2f & %d & %.2f & %.2f & %.2f\\\\"%( ils_min, ils_mean, ils_per_sd[i], ils_mean_time[i], lahc_min, lahc_mean, lahc_per_sd[i], lahc_mean_time[i] ) )


#ils_gap = np.array(ils_gap)
ils_per_sd = np.array(ils_per_sd)
ils_mean_time = np.array(ils_mean_time)
#lahc_gap = np.array(lahc_gap)
lahc_per_sd = np.array(lahc_per_sd)
lahc_mean_time = np.array(lahc_mean_time)

print("Max &  &  & %.2f & %.2f &  &  & %.2f & %.2f\\\\"%( ils_per_sd.max(), ils_mean_time.max(), lahc_per_sd.max(), lahc_mean_time.max() ) )
print("Mean &  &  & %.2f & %.2f &  &  & %.2f & %.2f\\\\"%( ils_per_sd.mean(), ils_mean_time.mean(), lahc_per_sd.mean(), lahc_mean_time.mean() ) )
print("Min &  &  & %.2f & %.2f &  &  & %.2f & %.2f\\\\"%( ils_per_sd.min(), ils_mean_time.min(), lahc_per_sd.min(), lahc_mean_time.min() ) )
