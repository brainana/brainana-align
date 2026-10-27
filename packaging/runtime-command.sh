# Sourced by the macOS launcher. Keep executable selection and launchd's environment
# in one place: launchd does not inherit the launcher's Electron environment.
brainana_select_runtime() {
  if [[ "${BRAINANA_DESKTOP_REMOTE:-0}" == "1" ]]; then
    NODE="${BRAINANA_DESKTOP_EXECUTABLE:-}"
    [[ "$NODE" == /* && -x "$NODE" ]] || return 33
    NODE_COMMAND=(/usr/bin/env ELECTRON_RUN_AS_NODE=1 "$NODE")
  else
    case "$HARDWARE_ARCH" in
      arm64) NODE="$RES/runtime/darwin-arm64/node" ;;
      x86_64) NODE="$RES/runtime/darwin-x64/node" ;;
      *) return 31 ;;
    esac
    [[ -x "$NODE" ]] || return 33
    NODE_COMMAND=("$NODE")
  fi
}
brainana_write_server_wrapper() {
  local destination="$1" log="$2" server="$3"
  shift 3
  {
    printf '#!/bin/bash\nexec '
    printf '%q ' "${NODE_COMMAND[@]}" "$server" "$@"
    printf '>> %q 2>&1 < /dev/null\n' "$log"
  } > "$destination"
  /bin/chmod 700 "$destination"
}
