class Corrade < Formula
  desc "C++11/C++14 multiplatform utility library"
  homepage "https://magnum.graphics/corrade/"
  url "https://github.com/mosra/corrade/archive/v2019.01.tar.gz"
  # wget https://github.com/mosra/corrade/archive/v2019.01.tar.gz -O - | sha256sum
  sha256 "67c813e8e2e687410ff2fac917d3c21d3c91d3e9c997a3d00fb78733ade1e13b"
  head "git://github.com/mosra/corrade.git"

  depends_on "cmake"

  def install
    system "mkdir build"
    cd "build" do
      system "cmake", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_INSTALL_PREFIX=#{prefix}", ".."
      system "cmake", "--build", "."
      system "cmake", "--build", ".", "--target", "install"
    end
  end
end
