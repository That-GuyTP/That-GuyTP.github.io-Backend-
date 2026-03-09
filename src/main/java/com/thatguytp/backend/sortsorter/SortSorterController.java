package com.thatguytp.backend.sortsorter;

import java.util.Map;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/sortsorter")
public class SortSorterController {

  private final SortSorterService service;

  public SortSorterController(SortSorterService service) {
    this.service = service;
  }

  @PostMapping("/sort")
  public ResponseEntity<?> sort(@RequestBody SortRequest request) {
    try {
      if (request == null) {
        throw new IllegalArgumentException("Request body is required.");
      }
      return ResponseEntity.ok(service.sort(request.words()));
    } catch (IllegalArgumentException ex) {
      return ResponseEntity.badRequest().body(Map.of("error", ex.getMessage()));
    }
  }
}
