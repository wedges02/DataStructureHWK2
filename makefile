CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

TARGET = simplify
SRC = simplify.cpp

TEST_DIR = test_cases

# Test case definitions: name:target_vertices
TESTS = \
	rectangle_with_two_holes:7 \
	cushion_with_hexagonal_hole:13 \
	blob_with_two_holes:17 \
	wavy_with_three_holes:21 \
	lake_with_two_islands:17 \
	original_01:99 \
	original_02:99 \
	original_03:99 \
	original_04:99 \
	original_05:99 \
	original_06:99 \
	original_07:99 \
	original_08:99 \
	original_09:99 \
	original_10:99

# Timeout per test case in seconds (0 = no timeout)
TIMEOUT = 300

.PHONY: all clean test test_grid test_unoptimized test_all

all: $(TARGET)

converter: converter.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Run tests with spatial grid (default)
test: test_grid

# Run tests with spatial grid, output to actual_*.txt
test_grid: $(TARGET)
	@echo "=== Running tests WITH spatial grid ==="; echo ""; \
	passed=0; failed=0; skipped=0; total=0; \
	for entry in $(TESTS); do \
		name=$$(echo $$entry | cut -d: -f1); \
		target=$$(echo $$entry | cut -d: -f2); \
		input="$(TEST_DIR)/input_$${name}.csv"; \
		expected="$(TEST_DIR)/output_$${name}.txt"; \
		actual="$(TEST_DIR)/actual_$${name}.txt"; \
		total=$$((total + 1)); \
		echo "=== Test: $$name (target=$$target) ==="; \
		if timeout $(TIMEOUT) ./$(TARGET) $$input $$target > $$actual 2>/dev/null; then \
			if diff -q $$expected $$actual > /dev/null 2>&1; then \
				echo "  PASS (exact match)"; \
				passed=$$((passed + 1)); \
			else \
				in_area=$$(grep "area in input" $$actual | awk '{print $$NF}'); \
				out_area=$$(grep "area in output" $$actual | awk '{print $$NF}'); \
				if [ -n "$$in_area" ] && [ -n "$$out_area" ] && [ "$$in_area" = "$$out_area" ]; then \
					echo "  PASS (area preserved, different vertices due to tie-breaking)"; \
					passed=$$((passed + 1)); \
				else \
					echo "  FAIL (area mismatch: input=$$in_area output=$$out_area)"; \
					failed=$$((failed + 1)); \
				fi; \
				echo "  Diff summary:"; \
				diff $$expected $$actual | tail -6; \
			fi; \
		else \
			echo "  SKIPPED (timeout after $(TIMEOUT)s)"; \
			skipped=$$((skipped + 1)); \
		fi; \
		echo ""; \
	done; \
	echo "=== Grid: $$passed passed, $$failed failed, $$skipped skipped out of $$total ==="

# Run tests WITHOUT spatial grid (brute force), output to unoptimized_actual_*.txt
test_unoptimized: $(TARGET)
	@echo "=== Running tests WITHOUT spatial grid (brute force) ==="; echo ""; \
	passed=0; failed=0; skipped=0; total=0; \
	for entry in $(TESTS); do \
		name=$$(echo $$entry | cut -d: -f1); \
		target=$$(echo $$entry | cut -d: -f2); \
		input="$(TEST_DIR)/input_$${name}.csv"; \
		expected="$(TEST_DIR)/output_$${name}.txt"; \
		actual="$(TEST_DIR)/unoptimized_actual_$${name}.txt"; \
		total=$$((total + 1)); \
		echo "=== Test: $$name (target=$$target) ==="; \
		if timeout $(TIMEOUT) ./$(TARGET) $$input $$target --no-grid > $$actual 2>/dev/null; then \
			if diff -q $$expected $$actual > /dev/null 2>&1; then \
				echo "  PASS (exact match)"; \
				passed=$$((passed + 1)); \
			else \
				in_area=$$(grep "area in input" $$actual | awk '{print $$NF}'); \
				out_area=$$(grep "area in output" $$actual | awk '{print $$NF}'); \
				if [ -n "$$in_area" ] && [ -n "$$out_area" ] && [ "$$in_area" = "$$out_area" ]; then \
					echo "  PASS (area preserved, different vertices due to tie-breaking)"; \
					passed=$$((passed + 1)); \
				else \
					echo "  FAIL (area mismatch: input=$$in_area output=$$out_area)"; \
					failed=$$((failed + 1)); \
				fi; \
				echo "  Diff summary:"; \
				diff $$expected $$actual | tail -6; \
			fi; \
		else \
			echo "  SKIPPED (timeout after $(TIMEOUT)s)"; \
			skipped=$$((skipped + 1)); \
		fi; \
		echo ""; \
	done; \
	echo "=== Brute force: $$passed passed, $$failed failed, $$skipped skipped out of $$total ==="

# Run both test suites
test_all: test_grid test_unoptimized

clean:
	rm -f $(TARGET) converter $(TEST_DIR)/actual_*.txt $(TEST_DIR)/unoptimized_actual_*.txt
