#include "simpleShell.h"

int main(){
  loadJsonFile("commands.json");

  history();

  return 0;
}