cmd_/home/Load.ko-driver/modules.order := {   echo /home/Load.ko-driver/helloworld.ko; :; } | awk '!x[$$0]++' - > /home/Load.ko-driver/modules.order
