################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_exp_common.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_exp_node.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_exp_transport.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_dl_list.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_eth_util.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_event_log.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_events.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_fmc_pkt.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_high.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_ltg.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_packet_types.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_queue.c \
C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_schedule.c 

OBJS += \
./wlan_mac_high_framework/wlan_exp_common.o \
./wlan_mac_high_framework/wlan_exp_node.o \
./wlan_mac_high_framework/wlan_exp_transport.o \
./wlan_mac_high_framework/wlan_mac_dl_list.o \
./wlan_mac_high_framework/wlan_mac_eth_util.o \
./wlan_mac_high_framework/wlan_mac_event_log.o \
./wlan_mac_high_framework/wlan_mac_events.o \
./wlan_mac_high_framework/wlan_mac_fmc_pkt.o \
./wlan_mac_high_framework/wlan_mac_high.o \
./wlan_mac_high_framework/wlan_mac_ltg.o \
./wlan_mac_high_framework/wlan_mac_packet_types.o \
./wlan_mac_high_framework/wlan_mac_queue.o \
./wlan_mac_high_framework/wlan_mac_schedule.o 

C_DEPS += \
./wlan_mac_high_framework/wlan_exp_common.d \
./wlan_mac_high_framework/wlan_exp_node.d \
./wlan_mac_high_framework/wlan_exp_transport.d \
./wlan_mac_high_framework/wlan_mac_dl_list.d \
./wlan_mac_high_framework/wlan_mac_eth_util.d \
./wlan_mac_high_framework/wlan_mac_event_log.d \
./wlan_mac_high_framework/wlan_mac_events.d \
./wlan_mac_high_framework/wlan_mac_fmc_pkt.d \
./wlan_mac_high_framework/wlan_mac_high.d \
./wlan_mac_high_framework/wlan_mac_ltg.d \
./wlan_mac_high_framework/wlan_mac_packet_types.d \
./wlan_mac_high_framework/wlan_mac_queue.d \
./wlan_mac_high_framework/wlan_mac_schedule.d 


# Each subdirectory must supply rules for building sources it contributes
wlan_mac_high_framework/wlan_exp_common.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_exp_common.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_exp_node.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_exp_node.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_exp_transport.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_exp_transport.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_dl_list.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_dl_list.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_eth_util.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_eth_util.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_event_log.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_event_log.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_events.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_events.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_fmc_pkt.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_fmc_pkt.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_high.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_high.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_ltg.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_ltg.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_packet_types.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_packet_types.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_queue.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_queue.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

wlan_mac_high_framework/wlan_mac_schedule.o: C:/Users/Ahmed.Youssef/Desktop/w3_802.11_EDK_v0.71_beta/SDK_Workspace/wlan_mac_shared/wlan_mac_high_framework/wlan_mac_schedule.c
	@echo Building file: $<
	@echo Invoking: MicroBlaze gcc compiler
	mb-gcc -Wall -O0 -g3 -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_high_sta\src\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_common\include" -I"C:\Users\Ahmed.Youssef\Desktop\w3_802.11_EDK_v0.71_beta\SDK_Workspace\wlan_mac_shared\wlan_mac_high_framework\include" -c -fmessage-length=0 -Wl,--no-relax -I../../wlan_bsp_cpu_high/mb_high/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.40.b -mno-xl-soft-mul -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '


