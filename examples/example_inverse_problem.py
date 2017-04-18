import numpy as np
from scipy import io
from scipy import linalg
from enthought.mayavi import mlab
from mesh import Mesh

cortex = Mesh("cortex.tri")
electrodes = np.loadtxt('eeg_channels_locations.txt')
squids = np.loadtxt('meg_channels_locations.squids')

G_eeg = io.loadmat('leadfield_eeg.mat')['linop']

chan_idx = 28

n_dipoles = G_eeg.shape[1]

###############################################################################
# Generate activation map with only one dipole active
x_simu = np.zeros(n_dipoles)
dipole_idx = 1000
x_simu[dipole_idx] = 1

# compute forward model
m = np.dot(G_eeg, x_simu)

# add measurement noise
m += 1e-8*np.random.randn(*m.shape)

# show topography
electrodes_mesh = Mesh("eeg_channels_mesh.tri")

mlab.figure(1)
mlab.clf()
electrodes_mesh.plot(opacity=1, scalars=m)

###############################################################################
# Run minimum norm
def minimum_norm(m, G, lambd):
    """Compute basic Minimum Norm solution
    x = G^T (G * G^T + lambda * I)^(-1) m
    """
    n_channels = G.shape[0]
    x = np.dot(G.T,
            linalg.solve(np.dot(G, G.T) + lambd * np.eye(n_channels), m))
    return x

lambd = 1e-20
x_estimated = minimum_norm(m, G_eeg, lambd)

# show source estimates
mlab.figure(2)
mlab.clf()
cortex.plot(opacity=1, scalars=x_estimated)
