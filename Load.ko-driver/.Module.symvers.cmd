cmd_/home/Load.ko-driver/Module.symvers := sed 's/\.ko$$/\.o/' /home/Load.ko-driver/modules.order | scripts/mod/modpost     -o /home/Load.ko-driver/Module.symvers -e -i Module.symvers   -T -
