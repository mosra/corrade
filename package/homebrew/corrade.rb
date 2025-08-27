class Corrade < Formula
  desc "C++11 multiplatform utility library"
  homepage "https://magnum.graphics/corrade/"
  # git describe origin/master, except the `v` prefix
  version "2020.06-2002-gf37591439"
  # Clone instead of getting an archive to have tags for version.h generation
  url "https://github.com/mosra/corrade.git", revision: "f37591439"
  head "https://github.com/mosra/corrade.git"

  depends_on "cmake" => :build

  def install
    system "mkdir build"
    cd "build" do
      system "cmake",
        *std_cmake_args,
        # Without this, ARM builds will try to look for dependencies in
        # /usr/local/lib and /usr/lib (which are the default locations) instead
        # of /opt/homebrew/lib which is dedicated for ARM binaries. Please
        # complain to Homebrew about this insane non-obvious filesystem layout.
        "-DCMAKE_INSTALL_NAME_DIR:STRING=#{lib}",
        ".."
      system "cmake", "--build", "."
      system "cmake", "--build", ".", "--target", "install"
    end
  end
end
