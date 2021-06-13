
.PHONY: all
all: run_alarm_test run_bargraph_test run_controller_test run_numerical_test run_sound_test run_state_tester_test run_button_test
	@echo "ALL SUCCESS"
# Includes come last so all is default target
include Makefile.*

sloc:
	@gcc -o ncsl ncsl.c
	@echo "Using LARS ncsl tool."
	@echo "Application code:"
	@./ncsl ../Core/Src/ventilator/*.c ../Core/Inc/ventilator/*.h
	@echo "Test Code:"
	@./ncsl *.c *.h
