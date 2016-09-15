# SMT

[![License](https://img.shields.io/badge/license-BSD-blue.svg)](LICENSE.md)

The [Spherical Mean Technique (SMT)](http://dx.doi.org/10.1002/mrm.25734) is a clinically feasible method for microscopic diffusion anisotropy imaging. The purpose is to map microscopic features unconfounded by the effects of fibre crossings and orientation dispersion, which are ubiquitous in the brain. This technique requires only an off-the-shelf diffusion sequence with two (or more) b-shells achievable on any standard MRI scanner, thus facilitating its widespread use in neuroscience research and clinical neurology. So far, SMT comes in two flavours, a microscopic tensor model and a simple multi-compartment model. This software was developed and is maintained by [Enrico Kaden](https://ekaden.github.io) from University College London.

## Building

SMT requires the following software (tested under Ubuntu Linux 14.04 LTS):
* [git](https://git-scm.com/) (version 1.9 or later)
* [cmake](https://cmake.org/) (version 2.8 or later)
* [GNU C/C++ compiler](https://gcc.gnu.org/) (version 4.9 or later), [Clang C/C++ compiler](http://clang.llvm.org/) (version 3.5 or later) or [Intel C/C++ compiler](https://software.intel.com/en-us/c-compilers/) (version 16.0 or later)

Clone the SMT repository and checkout the latest release:
```bash
git clone https://github.com/ekaden/smt.git
cd smt
git fetch -p
git checkout $(git describe --tags `git rev-list --tags --max-count=1`)
cd ..
```

Build the software:
```bash
mkdir smt-build
cd smt-build
cmake ../smt
make
```

If necessary, the C/C++ compiler can be set explicitly, for example:
```bash
cmake ../smt -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9
make
```

The SMT programs are located in the build directory.

## Microscopic diffusion tensor

This microscopic model estimates the microscopic fractional anisotropy and other microscopic diffusion coefficients in tissue with complex directional architecture as in the brain.

### Usage

```
fitmicrodt [options] <input> <output>
fitmicrodt (-h | --help)
fitmicrodt --licence
fitmicrodt --version
```

* `<input>` –– Input diffusion data set in NIfTI-1 format

* `<output>` –– Output parameter maps in NIfTI-1 format, containing:
  1. Longitudinal microscopic diffusivity
  2. Transverse microscopic diffusivity
  3. Microscopic fractional anisotropy
  4. Microscopic mean diffusivity
  5. Zero b-value image

### Options

* `--bvals <bvals>` –– Diffusion weighting factors (s/mm²), given in FSL format

* `--bvecs <bvecs>` –– Diffusion gradient directions, given in FSL format

* `--grads <grads>` –– Diffusion gradients (s/mm²), given in MRtrix format

* `--graddev <graddev>` –– Diffusion gradient deviation [default: none], provided as NIfTI-1 tensor volume

* `--mask <mask>` –– Foreground mask [default: none]. Values greater than zero are considered as foreground.

* `--rician <rician>` –– Rician noise [default: none]. SMT assumes Gaussian noise by default. Alternatively, a Rician noise model may be chosen, in which case the signal measurements are [adjusted](http://dx.doi.org/10.1002/mrm.25734) to reduce the Rician-noise induced bias. The noise level can be specified either globally using a scalar value or voxel by voxel using a NIfTI-1 image volume.

* `--maxdiff <maxdiff>` –– Maximum diffusivity (mm²/s) [default: 3.05e-3]. Typically the self/free-diffusion coefficient for a certain temperature is chosen.

* `--b0` –– Model-based estimation of the zero b-value signal. By default, the zero b-value signal is estimated as the mean over the measurements with zero b-value. If this option is set, the zero b-value signal is fitted using the microscopic diffusion model. This is also the default behaviour when measurements with zero b-value are not provided.

* `-h, --help` –– Help screen

* `--license` –– License information

* `--version` –– Software version

## Multi-compartment microscopic diffusion

This model provides estimates of microscopic features specific to the intra- and extra-neurite compartments unconfounded by the effects of the potentially complex fibre orientation distribution.

### Usage

```
fitmcmicro [options] <input> <output>
fitmcmicro (-h | --help)
fitmcmicro --licence
fitmcmicro --version
```

* `<input>` –– Input diffusion data set in NIfTI-1 format

* `<output>` –– Output parameter maps in NIfTI-1 format, containing:
  1. Intra-neurite volume fraction
  2. Intrinsic diffusivity
  3. Extra-neurite transverse microscopic diffusivity
  4. Extra-neurite microscopic mean diffusivity
  5. Zero b-value image

### Options

* `--bvals <bvals>` –– Diffusion weighting factors (s/mm²), given in FSL format

* `--bvecs <bvecs>` –– Diffusion gradient directions, given in FSL format

* `--grads <grads>` –– Diffusion gradients (s/mm²), given in MRtrix format

* `--graddev <graddev>` –– Diffusion gradient deviation [default: none], provided as NIfTI-1 tensor volume

* `--mask <mask>` –– Foreground mask [default: none]. Values greater than zero are considered as foreground.

* `--rician <rician>` –– Rician noise [default: none]. SMT assumes Gaussian noise by default. Alternatively, a Rician noise model may be chosen, in which case the signal measurements are [adjusted](http://dx.doi.org/10.1002/mrm.25734) to reduce the Rician-noise induced bias. The noise level can be specified either globally using a scalar value or voxel by voxel using a NIfTI-1 image volume.

* `--maxdiff <maxdiff>` –– Maximum diffusivity (mm²/s) [default: 3.05e-3]. Typically the self/free-diffusion coefficient for a certain temperature is chosen.

* `--b0` –– Model-based estimation of the zero b-value signal. By default, the zero b-value signal is estimated as the mean over the measurements with zero b-value. If this option is set, the zero b-value signal is fitted using the microscopic diffusion model. This is also the default behaviour when measurements with zero b-value are not provided.

* `-h, --help` –– Help screen

* `--license` –– License information

* `--version` –– Software version

## Citation

If you use this software, please cite:

* General Spherical Mean Technique (SMT)

Kaden E, Kruggel F, and Alexander DC: Quantitative Mapping of the Per-Axon Diffusion Coefficients in Brain White Matter. Magnetic Resonance in Medicine, vol. 75, pp. 1752–1763, 2016. DOI: [10.1002/mrm.25734](http://dx.doi.org/10.1002/mrm.25734)
<!-- Bibtex entry? -->

* Microscopic diffusion tensor model

Kaden E, Kruggel F, and Alexander DC: Quantitative Mapping of the Per-Axon Diffusion Coefficients in Brain White Matter. Magnetic Resonance in Medicine, vol. 75, pp. 1752–1763, 2016. DOI: [10.1002/mrm.25734](http://dx.doi.org/10.1002/mrm.25734)
<!-- Bibtex entry? -->

* Multi-compartment microscopic diffusion model

Kaden E, Kelm ND, Carson RP, Does MD, and Alexander DC: Multi-compartment microscopic diffusion imaging. NeuroImage, vol. 139, pp. 346–359, 2016. DOI: [10.1016/j.neuroimage.2016.06.002](http://dx.doi.org/10.1016/j.neuroimage.2016.06.002)
<!-- Bibtex entry? -->

## Supplementary material

* [ISMRM 2016 slides](docs/pdf/KadenISMRM2016.pdf)
* [OHBM 2016 poster](docs/pdf/KadenOHBM2016.pdf)

## License

SMT is released under the [BSD Two-Clause License](LICENSE.md).
