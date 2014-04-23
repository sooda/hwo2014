plot 'stderr.keimola' u 1:3 w lp t 'speed', 'stderr.keimola' u 1:5 w l t 'pieceidx',1-exp(-(x)/49.49), 'stderr.keimola' u 1:6 w p t 'angle', 'stderr.keimola' u 1:7 w l t 'angspd'
plot 'stderr.germany' u 1:3 w lp t 'speed', 'stderr.germany' u 1:5 w l t 'pieceidx',1-exp(-(x)/49.49), 'stderr.germany' u 1:6 w p t 'angle', 'stderr.germany' u 1:7 w l t 'angspd'
#plot 'stderr.usa' u 1:3 w lp t 'speed', 'stderr.usa' u 1:5 w l t 'pieceidx',1-exp(-(x)/49.49), 'stderr.usa' u 1:6 w p t 'angle', 'stderr.usa' u 1:7 w l t 'angspd'
