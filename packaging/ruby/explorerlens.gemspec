# frozen_string_literal: true

require_relative 'lib/explorerlens/version'

Gem::Specification.new do |spec|
  spec.name          = 'explorerlens'
  spec.version       = ExplorerLens::VERSION
  spec.authors       = ['ExplorerLens Project']
  spec.email         = []

  spec.summary       = 'GPU-accelerated thumbnail provider for 200+ file formats (Windows)'
  spec.description   = <<~DESC
    ExplorerLens provides GPU-accelerated thumbnail generation for 200+ file formats
    on Windows via the lens.exe CLI tool. This gem downloads and wraps lens.exe,
    enabling thumbnail generation from Ruby scripts and Rake tasks on Windows.
  DESC
  spec.homepage      = 'https://github.com/RajwanYair/ExplorerLens.io'
  spec.license       = 'MIT'

  spec.required_ruby_version = '>= 3.0'

  spec.metadata = {
    'homepage_uri'    => spec.homepage,
    'source_code_uri' => spec.homepage,
    'changelog_uri'   => "#{spec.homepage}/blob/main/CHANGELOG.md",
    'bug_tracker_uri' => "#{spec.homepage}/issues"
  }

  spec.files         = Dir['lib/**/*', 'exe/*', 'README.md', 'LICENSE', 'CHANGELOG.md']
  spec.bindir        = 'exe'
  spec.executables   = ['lens']
  spec.require_paths = ['lib']
end
