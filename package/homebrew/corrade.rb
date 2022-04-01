class Corrade < Formula
  desc "C++11/C++14 multiplatform utility library"
  homepage "https://magnum.graphics/corrade/"
  url "https://github.com/mosra/corrade/archive/v2020.06.tar.gz"
  # wget https://github.com/mosra/corrade/archive/v2020.06.tar.gz -O - | sha256sum
  sha256 "2a62492ccc717422b72f2596a3e1a6a105b9574aa9467917f12d19ef3aab1341"
  head "https://github.com/mosra/corrade.git"

  depends_on "cmake"

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
