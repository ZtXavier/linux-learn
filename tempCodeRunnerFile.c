if(sigprocmask(SIG_SETMASK,&oldmask,NULL) < 0){
    // my_err("sigprocmask",__LINE__);
    // }