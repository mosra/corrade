class Corrade < Formula
  desc "C++11/C++14 multiplatform utility library"
  homepage "https://magnum.graphics/corrade/"
  url "https://github.com/mosra/corrade/archive/v2020.06.tar.gz"
  # wget https://github.com/mosra/corrade/archive/v2020.06.tar.gz -O - | sha256sum
  sha256 "2a62492ccc717422b72f2596a3e1a6a105b9574aa9467917f12d19ef3aab1341"
  head "git://github.com/mosra/corrade.git"

  depends_on "cmake"

  def install
    system "mkdir build"
    cd "build" do
      system "cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_INSTALL_PREFIX=#{prefix}",
        ".."
      system "cmake", "--build", "."
      system "cmake", "--build", ".", "--target", "install"
    end
  end
end
