#include "logger_cpu.h"

void iniciar_logger(void){
    logger = log_create("./cfg/cpu.log","CPU",true,LOG_LEVEL_INFO);
}