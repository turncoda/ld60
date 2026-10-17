(
  "constexpr struct { SDL_Rect rect; int duration; } kFrames_\(.meta.image | split(".")[0])[] = {"
),
( [ .frames.[]
    | (.filename | split(".").[0] | gsub(" "; "_")) as $var_name
    | (.frame | to_entries | map(".\(.key) = \(.value)") | join(", ")) as $rect_fields
    | "  {\n    .rect = { \($rect_fields) },\n    .duration = \(.duration),\n  },"
  ] | join("\n")
),
(
  "};"
),
( [ (.meta.image | split(".")[0]) as $var_name
    | .meta.frameTags.[]
    | "static const char* kDirection_\($var_name) = \"\(.direction)\";"
  ] | join("\n")
),
( [ (.meta.image | split(".")[0]) as $var_name
    | .meta.slices.[].keys.[]
    | (.pivot | to_entries | map(".\(.key) = \(.value)") | join(", ")) as $fields
    | "static SDL_Point kPivot_\($var_name) = { \($fields) };"
  ] | join("\n")
)
