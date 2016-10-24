################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dispatch.c \
../src/http_response.c \
../src/main.c \
../src/platform.c \
../src/platform_fs.c \
../src/platform_gpio.c \
../src/prot_malloc.c \
../src/web_utils.c \
../src/webserver.c 

LD_SRCS += \
../src/lscript.ld 

OBJS += \
./src/dispatch.o \
./src/http_response.o \
./src/main.o \
./src/platform.o \
./src/platform_fs.o \
./src/platform_gpio.o \
./src/prot_malloc.o \
./src/web_utils.o \
./src/webserver.o 

C_DEPS += \
./src/dispatch.d \
./src/http_response.d \
./src/main.d \
./src/platform.d \
./src/platform_fs.d \
./src/platform_gpio.d \
./src/prot_malloc.d \
./src/web_utils.d \
./src/webserver.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -I../../standalone_bsp_0/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mno-xl-soft-div -mcpu=v8.50.c -mno-xl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


