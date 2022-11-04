#!/usr/bin/env bash

set -euo pipefail

#######################################################
# bump_lyft_support_rotation.sh
#
# Assigns the next person in the Lyft maintainers list.
#######################################################

# TODO(jpsim): Use a yaml parsing library if the format ever expands
# beyond its current very simple form.

function next_maintainer() {
  current="$1"
  file="$2"

  while IFS= read -r line; do
    maintainers+=("${line#"  - "}")
  done <<< "$(sed -n '/^  - /p' "$file")"

  for i in "${!maintainers[@]}"; do
    if [[ "${maintainers[$i]}" = "${current}" ]]; then
      last_index=$((${#maintainers[@]}-1))
      if [[ $i == "$last_index" ]]; then
        echo "${maintainers[0]}"
        break
      else
        echo "${maintainers[$((i + 1))]}"
        break
      fi
    fi
  done
}

function set_maintainer() {
  maintainer="$1"
  file="$2"

  echo "current: $maintainer" > "$file.tmp"
  tail -n +2 "$file" >> "$file.tmp"
  mv "$file.tmp" "$file"
}

maintainers_file=".github/lyft_maintainers.yml"
first_line="$(head -n 1 "$maintainers_file")"
previous=${first_line#"current: "}
next="$(next_maintainer "$previous" "$maintainers_file")"
set_maintainer "$next" "$maintainers_file"

echo "PREVIOUS_MAINTAINER=$previous" >> "$GITHUB_OUTPUT"
echo "NEXT_MAINTAINER=$next" >> "$GITHUB_OUTPUT"

echo "Lyft support maintainer changing from $previous to $next"
