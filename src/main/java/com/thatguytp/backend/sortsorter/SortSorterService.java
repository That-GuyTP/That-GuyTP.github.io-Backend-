package com.thatguytp.backend.sortsorter;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import org.springframework.stereotype.Service;

@Service
public class SortSorterService {

  private static final int MAX_SIZE = 100;
  private static final String SORT_TOKEN = "sort";

  public SortResponse sort(List<String> words) {
    if (words == null) {
      throw new IllegalArgumentException("Request body must include a words array.");
    }

    if (words.size() > MAX_SIZE) {
      throw new IllegalArgumentException("Maximum list size is 100 words.");
    }

    for (String word : words) {
      if (word == null) {
        throw new IllegalArgumentException("Words array cannot contain null values.");
      }
    }

    List<String> sortedWords = mergeSort(new ArrayList<>(words));
    List<SortedEntry> sortedEntries = sortedWords.stream()
      .map(word -> new SortedEntry(word, countSortMatches(word)))
      .toList();

    return new SortResponse(sortedWords, sortedEntries, words.size());
  }

  int countSortMatches(String value) {
    String lowerValue = value.toLowerCase(Locale.ROOT);
    int count = 0;

    for (int i = 0; i <= lowerValue.length() - SORT_TOKEN.length(); i++) {
      if (lowerValue.substring(i, i + SORT_TOKEN.length()).equals(SORT_TOKEN)) {
        count++;
      }
    }

    return count;
  }

  private List<String> mergeSort(List<String> items) {
    if (items.size() <= 1) {
      return items;
    }

    int middle = items.size() / 2;
    List<String> left = mergeSort(new ArrayList<>(items.subList(0, middle)));
    List<String> right = mergeSort(new ArrayList<>(items.subList(middle, items.size())));

    return merge(left, right);
  }

  private List<String> merge(List<String> left, List<String> right) {
    List<String> merged = new ArrayList<>(left.size() + right.size());
    int leftIndex = 0;
    int rightIndex = 0;

    while (leftIndex < left.size() && rightIndex < right.size()) {
      if (countSortMatches(left.get(leftIndex)) <= countSortMatches(right.get(rightIndex))) {
        merged.add(left.get(leftIndex));
        leftIndex++;
      } else {
        merged.add(right.get(rightIndex));
        rightIndex++;
      }
    }

    while (leftIndex < left.size()) {
      merged.add(left.get(leftIndex));
      leftIndex++;
    }

    while (rightIndex < right.size()) {
      merged.add(right.get(rightIndex));
      rightIndex++;
    }

    return merged;
  }
}
