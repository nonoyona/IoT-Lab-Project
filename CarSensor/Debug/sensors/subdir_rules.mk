################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
sensors/%.obj: ../sensors/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1200/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/bin/armcl" -mv7M3 --code_state=16 --float_support=vfplib -me --include_path="D:/Documents/Git/IoT-Lab-Project/CarSensor" --include_path="C:/ti/simplelink_cc13x0_sdk_4_20_02_07/source/ti/posix/ccs" --include_path="C:/ti/ccs1200/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/include" --define=DeviceFamily_CC13X0 --define=CCFG_FORCE_VDDR_HH=0 --define=SUPPORT_PHY_CUSTOM --define=SUPPORT_PHY_50KBPS2GFSK --define=SUPPORT_PHY_625BPSLRM --define=SUPPORT_PHY_5KBPSSLLR -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="sensors/$(basename $(<F)).d_raw" --obj_directory="sensors" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


