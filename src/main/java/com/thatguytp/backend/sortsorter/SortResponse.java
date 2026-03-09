package com.thatguytp.backend.sortsorter;

import java.util.List;

public record SortResponse(List<String> sortedWords, List<SortedEntry> sortedEntries, int inputSize) {
}
