# SPDX-FileCopyrightText: 2025 Davide Achilli
# SPDX-License-Identifier: Apache-2.0

project(
  'dastd',
  'cpp',
  license: 'Apache-2.0',
  default_options: [
    'cpp_std=c++20',
  ],
  meson_version: '>=0.61.0'
)

version = '1.0.03'
dastd_headers = []
subdir('src/dastd')
dastd_dep = declare_dependency(include_directories: 'src', version: version)
meson.override_dependency('dastd', dastd_dep)

