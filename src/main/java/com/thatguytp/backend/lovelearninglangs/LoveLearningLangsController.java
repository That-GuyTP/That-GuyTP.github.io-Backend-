package com.thatguytp.backend.lovelearninglangs;

import com.thatguytp.lovelearninglangs.api.ApiException;
import com.thatguytp.lovelearninglangs.api.Config;
import com.thatguytp.lovelearninglangs.api.ContentService;
import com.thatguytp.lovelearninglangs.api.ExerciseService;
import com.thatguytp.lovelearninglangs.api.RateLimitService;
import com.thatguytp.lovelearninglangs.api.TokenUtil;
import com.thatguytp.lovelearninglangs.api.UserService;
import jakarta.servlet.http.HttpServletRequest;
import java.io.IOException;
import java.time.Instant;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api")
public class LoveLearningLangsController {

  @GetMapping("/health")
  public Map<String, Object> health() {
    return Map.of(
      "ok",
      true,
      "service",
      "lovelearninglangs-java-api",
      "timestamp",
      Instant.now().toString()
    );
  }

  @GetMapping("/content/bootstrap")
  public ResponseEntity<?> bootstrap() {
    try {
      return ResponseEntity.ok(ContentService.loadBootstrapPayload());
    } catch (Exception ex) {
      return internalError();
    }
  }

  @PostMapping("/auth/register")
  public ResponseEntity<?> register(
    @RequestBody(required = false) Map<String, Object> body,
    HttpServletRequest request
  ) {
    try {
      UserService.purgeExpiredUsers();
      ResponseEntity<?> limitResponse = enforceSaveRateLimit(request);
      if (limitResponse != null) {
        return limitResponse;
      }

      JSONObject user = UserService.register(toJsonObject(body));
      JSONObject payload = new JSONObject();
      payload.put("token", TokenUtil.createToken(asString(user.get("id"))));
      payload.put("user", user);
      return ResponseEntity.status(201).body(payload);
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @PostMapping("/auth/login")
  public ResponseEntity<?> login(@RequestBody(required = false) Map<String, Object> body) {
    try {
      UserService.purgeExpiredUsers();
      JSONObject user = UserService.login(toJsonObject(body));
      JSONObject payload = new JSONObject();
      payload.put("token", TokenUtil.createToken(asString(user.get("id"))));
      payload.put("user", user);
      return ResponseEntity.ok(payload);
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @GetMapping("/auth/me")
  public ResponseEntity<?> me(HttpServletRequest request) {
    try {
      UserService.purgeExpiredUsers();
      String userId = requireAuthenticatedUser(request);
      JSONObject user = UserService.getUserById(userId);
      if (user == null) {
        throw new ApiException(401, "UNAUTHORIZED", "Please sign in to continue.");
      }

      JSONObject payload = new JSONObject();
      payload.put("user", user);
      payload.put("retentionDays", Config.USER_RETENTION_MS / (24L * 60L * 60L * 1000L));
      return ResponseEntity.ok(payload);
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @PostMapping("/progress/add-language")
  public ResponseEntity<?> addLanguage(
    @RequestBody(required = false) Map<String, Object> body,
    HttpServletRequest request
  ) {
    try {
      UserService.purgeExpiredUsers();
      String userId = requireAuthenticatedUser(request);
      ResponseEntity<?> limitResponse = enforceSaveRateLimit(request);
      if (limitResponse != null) {
        return limitResponse;
      }

      JSONObject payload = toJsonObject(body);
      String language = asString(payload.get("language")).toUpperCase(Locale.ROOT);

      JSONObject response = new JSONObject();
      response.put("user", UserService.addLanguage(userId, language));
      return ResponseEntity.ok(response);
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @PostMapping("/progress/complete-exercise")
  public ResponseEntity<?> completeExercise(
    @RequestBody(required = false) Map<String, Object> body,
    HttpServletRequest request
  ) {
    try {
      UserService.purgeExpiredUsers();
      String userId = requireAuthenticatedUser(request);
      ResponseEntity<?> limitResponse = enforceSaveRateLimit(request);
      if (limitResponse != null) {
        return limitResponse;
      }

      JSONObject payload = toJsonObject(body);
      String language = asString(payload.get("language")).toUpperCase(Locale.ROOT);
      double score = parseDouble(payload.get("score"), Double.NaN);
      return ResponseEntity.ok(UserService.completeExercise(userId, language, score));
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @GetMapping("/progress/review")
  public ResponseEntity<?> review(
    @RequestParam(name = "language", required = false) String languageRaw,
    HttpServletRequest request
  ) {
    try {
      UserService.purgeExpiredUsers();
      String userId = requireAuthenticatedUser(request);
      JSONObject user = UserService.getUserById(userId);
      if (user == null) {
        throw new ApiException(401, "UNAUTHORIZED", "Please sign in to continue.");
      }

      String language = asString(languageRaw).toUpperCase(Locale.ROOT);
      if (!Config.SUPPORTED_LANGUAGES.contains(language)) {
        throw new ApiException(
          400,
          "INVALID_LANGUAGE",
          "Language must be one of: " + String.join(", ", Config.SUPPORTED_LANGUAGES) + "."
        );
      }

      JSONObject courseProg = user.get("courseProg") instanceof JSONObject progress
        ? progress
        : new JSONObject();
      int level = Math.max(
        Config.COURSE_LEVEL_MIN,
        (int) Math.floor(parseDouble(courseProg.get(language), Config.COURSE_LEVEL_MIN))
      );

      JSONObject response = new JSONObject();
      response.put("language", language);
      response.put("level", level);
      response.put("reviewCards", ContentService.loadReviewCards(language, level));
      return ResponseEntity.ok(response);
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @PostMapping("/exercise/start")
  public ResponseEntity<?> startExercise(
    @RequestBody(required = false) Map<String, Object> body,
    HttpServletRequest request
  ) {
    try {
      UserService.purgeExpiredUsers();
      String userId = requireAuthenticatedUser(request);

      JSONObject payload = toJsonObject(body);
      String language = asString(payload.get("language")).toUpperCase(Locale.ROOT);
      int count = (int) Math.floor(parseDouble(payload.get("count"), 10.0));
      return ResponseEntity.ok(ExerciseService.startExercise(userId, language, count));
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  @PostMapping("/exercise/submit")
  public ResponseEntity<?> submitExercise(
    @RequestBody(required = false) Map<String, Object> body,
    HttpServletRequest request
  ) {
    try {
      UserService.purgeExpiredUsers();
      String userId = requireAuthenticatedUser(request);
      ResponseEntity<?> limitResponse = enforceSaveRateLimit(request);
      if (limitResponse != null) {
        return limitResponse;
      }

      JSONObject payload = toJsonObject(body);
      String exerciseId = asString(payload.get("exerciseId"));
      JSONObject answers = payload.get("answers") instanceof JSONObject values ? values : new JSONObject();
      return ResponseEntity.ok(ExerciseService.submitExercise(userId, exerciseId, answers));
    } catch (ApiException ex) {
      return apiError(ex);
    } catch (Exception ex) {
      return internalError();
    }
  }

  private String requireAuthenticatedUser(HttpServletRequest request) throws ApiException, IOException {
    String authHeader = request.getHeader("Authorization");
    if (authHeader == null || !authHeader.startsWith("Bearer ")) {
      throw new ApiException(401, "UNAUTHORIZED", "Please sign in to continue.");
    }

    String token = authHeader.substring(7).trim();
    String userId = TokenUtil.verifyAndExtractUserId(token);
    if (userId == null || UserService.getUserById(userId) == null) {
      throw new ApiException(401, "UNAUTHORIZED", "Please sign in to continue.");
    }
    return userId;
  }

  private ResponseEntity<?> enforceSaveRateLimit(HttpServletRequest request) throws IOException {
    String ipAddress = clientIp(request);
    RateLimitService.SaveCheck check = RateLimitService.checkAndRecordSave(ipAddress);
    if (check.isAllowed()) {
      return null;
    }

    JSONObject payload = new JSONObject();
    payload.put("error", "SAVE_RATE_LIMITED");
    payload.put("message", "Too many save operations from this IP. Please try again soon.");
    payload.put("retryAfterSeconds", check.getRetryAfterSeconds());

    return ResponseEntity
      .status(429)
      .header("Retry-After", String.valueOf(check.getRetryAfterSeconds()))
      .body(payload);
  }

  private ResponseEntity<JSONObject> apiError(ApiException ex) {
    JSONObject payload = new JSONObject();
    payload.put("error", ex.getErrorCode());
    payload.put("message", ex.getMessage());
    return ResponseEntity.status(ex.getStatusCode()).body(payload);
  }

  private ResponseEntity<JSONObject> internalError() {
    JSONObject payload = new JSONObject();
    payload.put("error", "INTERNAL_ERROR");
    payload.put("message", "An unexpected error occurred.");
    return ResponseEntity.status(500).body(payload);
  }

  private String clientIp(HttpServletRequest request) {
    String forwardedFor = request.getHeader("X-Forwarded-For");
    if (forwardedFor != null && !forwardedFor.isBlank()) {
      String[] parts = forwardedFor.split(",");
      if (parts.length > 0) {
        return parts[0].trim();
      }
    }

    String remoteAddress = request.getRemoteAddr();
    return remoteAddress == null ? "unknown" : remoteAddress;
  }

  private JSONObject toJsonObject(Map<String, Object> body) {
    JSONObject object = new JSONObject();
    if (body == null) {
      return object;
    }

    for (Map.Entry<String, Object> entry : body.entrySet()) {
      object.put(entry.getKey(), normalizeJsonValue(entry.getValue()));
    }
    return object;
  }

  private Object normalizeJsonValue(Object value) {
    if (value instanceof Map<?, ?> map) {
      JSONObject object = new JSONObject();
      for (Map.Entry<?, ?> entry : map.entrySet()) {
        if (entry.getKey() != null) {
          object.put(String.valueOf(entry.getKey()), normalizeJsonValue(entry.getValue()));
        }
      }
      return object;
    }

    if (value instanceof List<?> list) {
      JSONArray array = new JSONArray();
      for (Object item : list) {
        array.add(normalizeJsonValue(item));
      }
      return array;
    }

    return value;
  }

  private String asString(Object value) {
    return value == null ? "" : String.valueOf(value).trim();
  }

  private double parseDouble(Object value, double fallback) {
    if (value == null) {
      return fallback;
    }
    try {
      return Double.parseDouble(String.valueOf(value).trim());
    } catch (NumberFormatException ignored) {
      return fallback;
    }
  }
}
