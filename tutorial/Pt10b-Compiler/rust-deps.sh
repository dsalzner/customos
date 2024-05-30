#!/bin/bash

# -- set environment
source "${HOME}/.cargo/env"

# -- install rustup and rustc, if missing
if ! command -v rustup &> /dev/null
then
  echo "Install rustup"
  curl https://sh.rustup.rs -sSf | sh -s -- -y
fi

# -- configure rustup
rustup component add rust-src

# -- install cargo, if missing
aptinstall() {
  status="$(dpkg-query -W --showformat='${db:Status-Status}' "$1" 2>&1)"
  if [ ! $? = 0 ] || [ ! "$status" = "installed" ]; then
    echo "Install $1"
    sudo apt -y install $1
  fi
}
aptinstall "cargo"
