# kate: indent-width 2;

class Corrade < Formula
  desc "C++11/C++14 multiplatform utility library"
  homepage "http://magnum.graphics/corrade/"
  url "https://github.com/mosra/corrade/archive/v2018.02.tar.gz"
  sha256 "6f05160b24dd34cdbd2b30d72972c750f20dd0982dd647780fb55e710626c4cc"
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
