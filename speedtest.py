# speed evaluation from log files
# a small error accumulates to 0.0003, maybe from number inaccuracy in log file
# or the drag is affected to v^2 instead of v
import sys

# power and drag on a specific thrust at the beginning of the race
def compute_p_d(thrust, x1, x2):
    # power adds to the speed; v1 = x1 - x0
    # also a = v1 as the car starts from zero
    power = x1 / thrust
    # speed decreases
    v2 = x2 - x1
    # v2 = drag * v1 + a
    drag = (v2 - x1) / x1
    return (power, drag)

def throttle(i):
    estim_interval = 10 * 60
    speed = min((i // estim_interval + 1) / 10.0, 0.7)
    return speed if i%estim_interval < estim_interval/2 else 0.0


def main():
    if len(sys.argv) != 2:
        print "need stderr param"
        return
    sim_end = 5000
    thrust = 0.1
    # magic values from a log; usually something like 0.02 0.0596
    lines = open(sys.argv[1]).readlines()[1:3]
    x1 = float(lines[0].split()[1])
    x2 = float(lines[1].split()[1])
    a, d = compute_p_d(thrust, x1, x2)
    x = 0
    v = 0
    for i in range(sim_end):
        print i, x, v
        thrust = throttle(i)
        v = d * v + a * thrust
        x += v

if __name__ == "__main__":
    main()
