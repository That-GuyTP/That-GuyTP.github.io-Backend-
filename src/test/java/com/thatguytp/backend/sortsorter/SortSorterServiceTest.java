package com.thatguytp.backend.sortsorter;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;
import java.util.List;
import org.junit.jupiter.api.Test;

class SortSorterServiceTest {

  private final SortSorterService service = new SortSorterService();

  @Test
  void sortOrdersWordsBySortCountAndPreservesTies() {
    List<String> input = List.of("sort", "assorted", "banana", "aSORTb", "SortSorter");

    SortResponse response = service.sort(input);

    assertEquals(List.of("banana", "sort", "assorted", "aSORTb", "SortSorter"), response.sortedWords());
    assertEquals(5, response.inputSize());
    assertEquals(0, response.sortedEntries().get(0).sortCount());
    assertEquals(2, response.sortedEntries().get(4).sortCount());
  }

  @Test
  void sortRejectsNullWords() {
    List<String> input = Arrays.asList("sort", null);
    assertThrows(IllegalArgumentException.class, () -> service.sort(input));
  }

  @Test
  void sortRejectsOversizedInput() {
    List<String> oversizedInput = java.util.stream.IntStream.range(0, 101)
      .mapToObj(i -> "word" + i)
      .toList();

    assertThrows(IllegalArgumentException.class, () -> service.sort(oversizedInput));
  }
}