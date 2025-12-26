pipeline {
    agent {
        docker {
            image 'alpine:3.21'
            // Монтування сокета Docker для можливості запуску Docker-команд всередині контейнера (DinD)
            args '-u root:root -v /var/run/docker.sock:/var/run/docker.sock'
        }
    }

    triggers {
        cron('H/30 * * * *')    // Періодичний запуск (кожні 30 хв)
        pollSCM('H/2 * * * *') // Опитування репозиторію на наявність змін (кожні 2 хв)
    }

    environment {
        DOCKER_IMAGE = 'artiprice/lab4-jenkins'
        DOCKER_CREDS_ID = 'docker-creds'
    }
    
    options {
        timestamps() // Додавання часових міток у логи
    }

    stages {
        
        stage('Check SCM') {
            steps {
                checkout scm
            }
        }
        
        stage('Update alpine repository') {
            steps {
                sh '''
                    apk update
                '''
            }
        }

        stage('Install deps') {
            steps {
                sh '''
                    apk add --no-cache cmake make gcc g++ libc-dev bash
                '''
            }
        }

        stage('Build vendors') {
            steps {
                // Компіляція сторонніх бібліотек під архітектуру Alpine Linux
                sh 'make vendor-build'
                sh 'ls vendors/fmt/build'
                sh 'ls vendors/gtest/build'
            }
        }

        stage('Build test binaries') {
            steps {
                sh 'make build-unit'
            }
        }
        
        stage('Test') {
            steps {
                sh 'make run-unit'
            }
            post {
                always {
                    // Публікація звітів JUnit незалежно від результату тестів
                    junit 'build/test-results/unit.xml'
                }
                success {
                    echo 'Application testing successfully completed'
                }
                failure {
                    echo 'Oh nooooo!!! Tests failed!'
                }
            }
        }

        stage('Build') {
            steps {
                sh 'make build'
            }
        }

        stage('Archive') {
            steps {
                // Збереження бінарного файлу як артефакту Jenkins
                archiveArtifacts artifacts: 'build/bin/app', fingerprint: true
            }
        }

        stage('Install docker') {
            steps {
                // Встановлення Docker CLI всередині агента для взаємодії з хостом
                sh '''
                    apk add --no-cache docker
                '''
            }
        }
        
        stage('Build Docker Image') {
            steps {
                script {
                    sh 'docker build -t ${DOCKER_IMAGE}:${BUILD_NUMBER} .'
                    sh 'docker tag ${DOCKER_IMAGE}:${BUILD_NUMBER} ${DOCKER_IMAGE}:latest'
                }
            }
        }
        
        stage('Push to Docker Hub') {
            steps {
                script {
                    // Безпечне використання облікових даних
                    withCredentials([usernamePassword(credentialsId: DOCKER_CREDS_ID, passwordVariable: 'DOCKER_PASS', usernameVariable: 'DOCKER_USER')]) {
                        sh 'echo $DOCKER_PASS | docker login -u $DOCKER_USER --password-stdin'
                        sh 'docker push ${DOCKER_IMAGE}:${BUILD_NUMBER}'
                        sh 'docker push ${DOCKER_IMAGE}:latest'
                    }
                }
            }
        }
        
        stage('Cleanup') {
             steps {
                 // Видалення локальних образів для звільнення місця
                 sh 'docker rmi ${DOCKER_IMAGE}:${BUILD_NUMBER} || true'
                 sh 'docker rmi ${DOCKER_IMAGE}:latest || true'
             }
        }
    }
}
