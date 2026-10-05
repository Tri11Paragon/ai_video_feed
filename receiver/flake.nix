{
  description = "Python development environment";

  inputs = {
#    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
	nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";

      pkgs = import nixpkgs {
        inherit system;
        config = {
            allowUnfree = true;
        };
      };


      makePython = pkgs.python314.withPackages (pythonPackages: [
          pythonPackages.pybox2d
          pythonPackages.mypy
          pythonPackages.numpy
          pythonPackages.pandas
          pythonPackages.scikit-learn
          pythonPackages.matplotlib
          pythonPackages.seaborn
          pythonPackages.tqdm
          pythonPackages.python-dotenv
          pythonPackages.openai
          pythonPackages.requests
          pythonPackages.mysql-connector
          pythonPackages.psycopg2
          pythonPackages.fastapi
          pythonPackages.uvicorn
        ]);

      makeShell = {
        python
      }:
        pkgs.mkShell {
          packages = [
            python
          ];

          shellHook = ''
            echo "Python: $(python --version)"
            exec zsh
          '';
        };
    in
    {
      devShells.${system} = {
            default = makeShell {
              python = makePython;
            };
      };
    };
}
