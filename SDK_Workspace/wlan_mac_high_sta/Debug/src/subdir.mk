################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/wlan_exp_node_sta.c \
../src/wlan_mac_sta.c \
../src/wlan_mac_sta_uart_menu.c 

LD_SRCS += \
../src/lscript.ld 

OBJS += \
./src/wlan_exp_node_sta.o \
./src/wlan_mac_sta.o \
./src/wlan_mac_sta_uart_menu.o 

C_DEPS += \
./src/wlan_exp_node_sta.d \
./src/wlan_mac_sta.d \
./src/wlan_mac_sta_uart_menu.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '


