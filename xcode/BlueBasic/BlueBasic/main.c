//
//  main.c
//  BlueBasic
//
//  Created by tim on 7/13/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#include <stdio.h>
#include "os.h"

extern void interpreter_setup(void);
extern void interpreter_loop(void);

int main(int argc, const char * argv[])
{
  interpreter_setup();
  interpreter_loop();
  
  return 0;
}

