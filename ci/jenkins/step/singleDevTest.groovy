timeout(time: 60, unit: 'MINUTES') {
    dir ("tests/milvus_python_test") {
        // sh 'python3 -m pip install -r requirements.txt -i http://pypi.douban.com/simple --trusted-host pypi.douban.com'
        sh 'python3 -m pip install -r requirements.txt'
        sh 'python3 -m pip install git+https://github.com/BossZou/pymilvus.git@nns'
        sh "pytest . --alluredir=\"test_out/dev/single/sqlite\" --level=1 --ip ${env.HELM_RELEASE_NAME}.milvus.svc.cluster.local"
    }

    // mysql database backend test
    // load "ci/jenkins/step/cleanupSingleDev.groovy"

    // if (!fileExists('milvus-helm')) {
    //     dir ("milvus-helm") {
    //         checkout([$class: 'GitSCM', branches: [[name: "master"]], userRemoteConfigs: [[url: "https://github.com/milvus-io/milvus-helm.git", name: 'origin', refspec: "+refs/heads/master:refs/remotes/origin/master"]]])
    //     }
    // }
    // dir ("milvus-helm") {
    //     sh "helm install --wait --timeout 300s --set image.repository=registry.zilliz.com/milvus/engine --set image.tag=${DOCKER_VERSION} --set image.pullPolicy=Always --set service.type=ClusterIP -f ci/db_backend/mysql_${BINARY_VERSION}_values.yaml -f ci/filebeat/values.yaml --namespace milvus ${env.HELM_RELEASE_NAME} ."
    // }
    // dir ("tests/milvus_python_test") {
    //     sh "pytest . --alluredir=\"test_out/dev/single/mysql\" --level=1 --ip ${env.HELM_RELEASE_NAME}.milvus.svc.cluster.local"
    // }
}
