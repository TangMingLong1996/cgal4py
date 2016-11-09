r"""Tests for parallel implementation of triangulations."""
import numpy as np
from cgal4py import parallel, delaunay
from test_cgal4py import make_test
from test_delaunay2 import pts as pts2
np.random.seed(10)


def test_ParallelLeaf_2D():
    pts, tree = make_test(0, 2)
    left_edges = np.vstack([leaf.left_edge for leaf in tree.leaves])
    right_edges = np.vstack([leaf.right_edge for leaf in tree.leaves])
    out = []
    pleaves = []
    for i, leaf in enumerate(tree.leaves):
        assert(leaf.id == i)
        pleaf = parallel.ParallelLeaf(leaf, left_edges, right_edges)
        pleaf.tessellate(pts2)
        pleaves.append(pleaf)
        out.append(pleaf.outgoing_points())
    pleaves[0].incoming_points(1, out[1][0][0], out[1][1], out[1][2],
                               out[1][3], pts[out[1][0][0], :])
    pleaves[0].incoming_points(0, out[0][0][0], out[0][1], out[0][2],
                               out[0][3], pts[out[0][0][0], :])


def test_ParallelLeaf_3D():
    pts, tree = make_test(0, 3)
    left_edges = np.vstack([leaf.left_edge for leaf in tree.leaves])
    right_edges = np.vstack([leaf.right_edge for leaf in tree.leaves])
    out = []
    pleaves = []
    for i, leaf in enumerate(tree.leaves):
        assert(leaf.id == i)
        pleaf = parallel.ParallelLeaf(leaf, left_edges, right_edges)
        pleaf.tessellate(pts)
        pleaves.append(pleaf)
        out.append(pleaf.outgoing_points())
    pleaves[0].incoming_points(1, out[1][0][0], out[1][1], out[1][2],
                               out[1][3], pts[out[1][0][0], :])
    pleaves[0].incoming_points(0, out[0][0][0], out[0][1], out[0][2],
                               out[0][3], pts[out[0][0][0], :])


def test_ParallelLeaf_periodic_2D():
    pts, tree = make_test(0, 2, periodic=True)
    left_edges = np.vstack([leaf.left_edge for leaf in tree.leaves])
    right_edges = np.vstack([leaf.right_edge for leaf in tree.leaves])
    out = []
    pleaves = []
    for i, leaf in enumerate(tree.leaves):
        assert(leaf.id == i)
        pleaf = parallel.ParallelLeaf(leaf, left_edges, right_edges)
        pleaf.tessellate(pts)
        pleaves.append(pleaf)
        out.append(pleaf.outgoing_points())
    pleaves[0].incoming_points(1, out[1][0][0], out[1][1], out[1][2],
                               out[1][3], pts[out[1][0][0], :])
    pleaves[0].incoming_points(0, out[0][0][0], out[0][1], out[0][2],
                               out[0][3], pts[out[0][0][0], :])


def test_ParallelLeaf_periodic_3D():
    pts, tree = make_test(0, 3, periodic=True)
    left_edges = np.vstack([leaf.left_edge for leaf in tree.leaves])
    right_edges = np.vstack([leaf.right_edge for leaf in tree.leaves])
    out = []
    pleaves = []
    for i, leaf in enumerate(tree.leaves):
        assert(leaf.id == i)
        pleaf = parallel.ParallelLeaf(leaf, left_edges, right_edges)
        pleaf.tessellate(pts)
        pleaves.append(pleaf)
        out.append(pleaf.outgoing_points())
    pleaves[0].incoming_points(1, out[1][0][0], out[1][1], out[1][2],
                               out[1][3], pts[out[1][0][0], :])
    pleaves[0].incoming_points(0, out[0][0][0], out[0][1], out[0][2],
                               out[0][3], pts[out[0][0][0], :])


def test_ParallelVoronoiVolumes_2D():
    # Small test with known solution
    pts, tree = make_test(0, 2)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 2, nleaves=2)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 2, leafsize=8)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 4)
    for i in range(pts.shape[0]):
        print v_seri[i], v_para[i]
    assert(np.allclose(v_seri, v_para))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 2, nleaves=8)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 8)
    assert(np.allclose(v_seri, v_para))


def test_ParallelVoronoiVolumes_3D():
    # Small test with known solution
    pts, tree = make_test(0, 3)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 3, nleaves=2)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 3, leafsize=8)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 4)
    assert(np.allclose(v_seri, v_para))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 3, nleaves=8)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 8)
    assert(np.allclose(v_seri, v_para))


def test_ParallelVoronoiVolumes_2D_periodic():
    # Small test with known solution
    pts, tree = make_test(0, 2, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 2, nleaves=2, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 2, leafsize=8, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 4)
    assert(np.allclose(v_seri, v_para))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 2, nleaves=8, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 8)
    assert(np.allclose(v_seri, v_para))


def test_ParallelVoronoiVolumes_3D_periodic():
    # Small test with known solution
    pts, tree = make_test(0, 3, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 3, nleaves=2, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 2)
    assert(np.allclose(v_seri, v_para))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 3, leafsize=8, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 4)
    assert(np.allclose(v_seri, v_para))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 3, nleaves=8, periodic=True)
    v_seri = delaunay.VoronoiVolumes(pts)
    v_para = parallel.ParallelVoronoiVolumes(pts, tree, 8)
    assert(np.allclose(v_seri, v_para))


def test_ParallelDelaunay_2D():
    # Small test with known solution
    pts, tree = make_test(0, 2)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 2, nleaves=2)
    assert(tree.num_leaves == 2)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    T_seri = delaunay.Delaunay(pts)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 2, leafsize=8)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 4)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 2, nleaves=8)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 8)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(T_para.is_equivalent(T_seri))
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    # Large test on 8 processors
    # pts, tree = make_test(1e7, 2, nleaves=10)
    # T_seri = delaunay.Delaunay(pts)
    # T_para = parallel.ParallelDelaunay(pts, tree, 10)
    # c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    # c_para, n_para, inf_para = T_para.serialize(sort=True)
    # assert(T_para.is_equivalent(T_seri))
    # assert(np.all(c_seri == c_para))
    # assert(np.all(n_seri == n_para))
    # for name, T in zip(['Parallel','Serial'],[T_para, T_seri]):
    #     print(name)
    #     print('\t verts', T.num_verts, T.num_finite_verts,
    #           T.num_infinite_verts)
    #     print('\t cells', T.num_cells, T.num_finite_cells,
    #           T.num_infinite_cells)
    #     print('\t edges', T.num_edges, T.num_finite_edges,
    #           T.num_infinite_edges)


def test_ParallelDelaunay_3D():
    # Small test with known solution
    pts, tree = make_test(0, 3)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 3)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    T_seri = delaunay.Delaunay(pts)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 3, leafsize=8)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 4)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 3, nleaves=8)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 8)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # for name, T in zip(['Parallel','Serial'],[T_para, T_seri]):
    #     print(name)
    #     print('\t verts', T.num_verts, T.num_finite_verts,
    #           T.num_infinite_verts)
    #     print('\t cells', T.num_cells, T.num_finite_cells,
    #           T.num_infinite_cells)
    #     print('\t edges', T.num_edges, T.num_finite_edges,
    #           T.num_infinite_edges)
    #     print('\t facets', T.num_facets, T.num_finite_facets,
    #           T.num_infinite_facets


def test_ParallelDelaunay_periodic_2D():
    # Small test with known solution
    pts, tree = make_test(0, 2, periodic=True)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 2, periodic=True)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    T_seri = delaunay.Delaunay(pts)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Medium test on 4 processors
    pts, tree = make_test(4*4*2, 2, leafsize=8, periodic=True)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 4)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 2, nleaves=8, periodic=True)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 8)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # for name, T in zip(['Parallel','Serial'],[T_para, T_seri]):
    #     print(name)
    #     print('\t verts', T.num_verts, T.num_finite_verts,
    #           T.num_infinite_verts)
    #     print('\t cells', T.num_cells, T.num_finite_cells,
    #           T.num_infinite_cells)
    #     print('\t edges', T.num_edges, T.num_finite_edges,
    #           T.num_infinite_edges)


def test_ParallelDelaunay_periodic_3D():
    # Small test with known solution
    pts, tree = make_test(0, 3, periodic=True)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Larger random test on 2 processors
    pts, tree = make_test(1000, 3, periodic=True)
    T_para = parallel.ParallelDelaunay(pts, tree, 2)
    T_seri = delaunay.Delaunay(pts)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # Large test on 8 processors
    pts, tree = make_test(1e5, 3, nleaves=8, periodic=True)
    T_seri = delaunay.Delaunay(pts)
    T_para = parallel.ParallelDelaunay(pts, tree, 8)
    c_seri, n_seri, inf_seri = T_seri.serialize(sort=True)
    c_para, n_para, inf_para = T_para.serialize(sort=True)
    assert(np.all(c_seri == c_para))
    assert(np.all(n_seri == n_para))
    assert(T_para.is_equivalent(T_seri))
    # for name, T in zip(['Parallel','Serial'],[T_para, T_seri]):
    #     print(name)
    #     print('\t verts', T.num_verts, T.num_finite_verts,
    #           T.num_infinite_verts)
    #     print('\t cells', T.num_cells, T.num_finite_cells,
    #           T.num_infinite_cells)
    #     print('\t edges', T.num_edges, T.num_finite_edges,
    #           T.num_infinite_edges)
    #     print('\t facets', T.num_facets, T.num_finite_facets,
    #           T.num_infinite_facets

# def test_DelaunayProcess2():
#     pts, tree = make_test(0, 2)
#     idxArray = mp.RawArray(ctypes.c_ulonglong, tree.idx.size)
#     ptsArray = mp.RawArray('d',pts.size)
#     memoryview(idxArray)[:] = tree.idx
#     memoryview(ptsArray)[:] = pts
#     left_edges = np.vstack([leaf.left_edge for leaf in tree.leaves])
#     right_edges = np.vstack([leaf.right_edge for leaf in tree.leaves])
#     leaves = tree.leaves
#     nproc = 2 # len(leaves)
#     count = [mp.Value('i',0),mp.Value('i',0),mp.Value('i',0)]
#     lock = mp.Condition()
#     queues = [mp.Queue() for _ in xrange(nproc+1)]
#     in_pipes = [None for _ in xrange(nproc)]
#     out_pipes = [None for _ in xrange(nproc)]
#     for i in range(nproc):
#         out_pipes[i],in_pipes[i] = mp.Pipe(True)
#     # Split leaves
#     task2leaves = [[] for _ in xrange(nproc)]
#     for leaf in leaves:
#         task = leaf.id % nproc
#         task2leaves[task].append(leaf)
#     # Create processes & tessellate
#     processes = []
#     for i in xrange(nproc):
#         P = parallel.DelaunayProcess('triangulate', i, task2leaves[i],
#                                      ptsArray, idxArray,
#                                      left_edges, right_edges,
#                                      queues, lock, count, in_pipes[i])
#         processes.append(P)
#     # Split
#     P1, P2 = processes[0], processes[1]
#     # Do partial run on 1
#     P1.tessellate_leaves()
#     P1.outgoing_points()
#     # Full run on 2
#     P2.run()
#     # Finish on 1
#     i,j,arr,ln,rn = queues[0].get()
#     queues[0].put((i,j,np.array([]),ln,rn))
#     P1.incoming_points()
#     P1.enqueue_triangulation()
#     time.sleep(0.01)
